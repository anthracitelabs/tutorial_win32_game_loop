For a custom 2D platformer engine with fast-moving entities, combine axis-separated AABB resolution with velocity sub-stepping as your primary tilemap collision pipeline. It avoids the corner-snagging bugs of swept Minkowski math on multi-tile grids while giving you deterministic control over platformer physics.

1. Sub-Stepping Loop

Fast projectiles or dashing characters can skip over single tiles in one frame. Divide the frame's displacement into small increments—typically capped at half the entity's smallest dimension (or half a tile width)—and run the collision solver for each step.
* Step Count: steps = ceil(max(abs(dx), abs(dy)) / maxStepSize)
* Benefit: Guarantees an entity never moves far enough in one iteration to miss a solid grid cell.

2. Axis-Separated Grid Resolution

For each sub-step, resolve the X-axis completely before moving on the Y-axis:
1. Move X: Add stepX to the entity's X position.
2. Query Overlapping Tiles: Compute the min/max tile indices overlapped by the entity's AABB (floor(min / tileSize) to floor((max - epsilon) / tileSize)).
3. Resolve X: If any overlapped tile is solid, snap the entity's leading edge flush against the tile boundary (tileX * tileSize or (tileX + 1) * tileSize) and zero out X velocity.
4. Move & Resolve Y: Repeat the exact same process for stepY using the updated X position.

3. Essential Platformer Edge Cases

* Epsilon Inset: Subtract a tiny epsilon (e.g., 0.001f) from the max bounds when calculating tile indices so an entity resting flush at x = 32.0 on a 32px grid doesn't falsely register an overlap with tile index 1.
* Internal Edge Snags: Resolving X and Y separately naturally prevents an entity from catching on flat seams between adjacent floor or wall tiles.
* Slopes & One-Way Platforms: Handle one-way platforms only during downward Y resolution when the entity's previous bottom edge was above the tile's top edge. For slopes, evaluate surface height y(x) at the entity's horizontal center or leading edge during the Y pass.

~~~~ cpp
struct AABB {
    float x, y, w, h; // Top-left position and size
};

void MoveAndCollide(AABB& box, float& vx, float& vy, float dt, float tileSize) {
    float dx = vx * dt;
    float dy = vy * dt;

    float maxStep = std::min(box.w, box.h) * 0.5f;
    int steps = std::max(1, (int)std::ceil(std::max(std::abs(dx), std::abs(dy)) / maxStep));

    float stepX = dx / steps;
    float stepY = dy / steps;
    const float eps = 0.001f;

    for (int s = 0; s < steps; ++s) {
        // 1. Resolve X
        box.x += stepX;
        int minTX = (int)std::floor(box.x / tileSize);
        int maxTX = (int)std::floor((box.x + box.w - eps) / tileSize);
        int minTY = (int)std::floor(box.y / tileSize);
        int maxTY = (int)std::floor((box.y + box.h - eps) / tileSize);

        for (int ty = minTY; ty <= maxTY; ++ty) {
            for (int tx = minTX; tx <= maxTX; ++tx) {
                if (IsTileSolid(tx, ty)) {
                    if (stepX > 0.0f) box.x = tx * tileSize - box.w;
                    else if (stepX < 0.0f) box.x = (tx + 1) * tileSize;
                    vx = 0.0f;
                    stepX = 0.0f;
                }
            }
        }

        // 2. Resolve Y
        box.y += stepY;
        minTX = (int)std::floor(box.x / tileSize);
        maxTX = (int)std::floor((box.x + box.w - eps) / tileSize);
        minTY = (int)std::floor(box.y / tileSize);
        maxTY = (int)std::floor((box.y + box.h - eps) / tileSize);

        for (int ty = minTY; ty <= maxTY; ++ty) {
            for (int tx = minTX; tx <= maxTX; ++tx) {
                if (IsTileSolid(tx, ty)) {
                    if (stepY > 0.0f) box.y = ty * tileSize - box.h;
                    else if (stepY < 0.0f) box.y = (ty + 1) * tileSize;
                    vy = 0.0f;
                    stepY = 0.0f;
                }
            }
        }
    }
}
~~~~

## Moving Platforms

To handle moving platforms in an axis-separated tilemap engine, update platforms first to carry riders and push intersecting entities, then run the entity's standard tilemap collision pass. Doing this in two distinct phases prevents jittery rider detachment and ensures entities pushed by a platform still collide cleanly with solid world tiles.


1. Frame Update Order

Run your physics step in this exact sequence every frame:
1. Move Platforms: Compute each platform's frame delta (platformDx, platformDy).
2. Apply Rider & Push Displacement: Before updating the platform's final world position (or immediately after, using its pre-move bounds), identify riders and pushed entities, and move them through the tilemap.
3. Move Entities: Run normal gravity, player input, and MoveAndCollide against both static tiles and moving platforms.

2. Riding vs. Pushing

Before moving a platform by (dx, dy), check every dynamic entity against the platform's pre-move AABB:
* Pushing (Overlap after platform moves): If the platform's new AABB overlaps an entity, push the entity along the platform's movement axis until it is flush with the platform's leading edge (platform.x + platform.w or platform.x - entity.w).
* Carrying (Standing on top): If the entity is grounded on top of the platform (abs((entity.y + entity.h) - platform.y) <= eps and horizontal bounds overlap), carry the entity by the platform's full (dx, dy)—unless the entity was already pushed on that axis.

3. Resolving Against Static Tiles (Crush Prevention)

Never blindly add platformDx or platformDy directly to an entity's coordinates. Instead, pass the carry/push delta into your tilemap collision solver (MoveAndCollideWithTiles) so the carried entity stops against static walls or ceilings.
* Wall Scraping: If a horizontal platform carries a player into a static wall, the tilemap solver stops the player flush against the wall while the platform slides underneath them.
* Crush Detection: After pushing an entity out of a moving platform and resolving against static tiles, check if the entity still overlaps the platform's new AABB. If it does, the entity is pinned between the platform and a solid tile—trigger your crush callback (damage, squish, or teleport).

~~~~ cpp
struct Platform {
    AABB box;
    float vx, vy;
};

void UpdatePlatforms(std::vector<Platform>& platforms, std::vector<Entity>& entities, float dt, float tileSize) {
    const float eps = 0.01f;

    for (auto& plat : platforms) {
        float dx = plat.vx * dt;
        float dy = plat.vy * dt;

        // 1. Move X and check riders/pushes
        AABB oldBox = plat.box;
        plat.box.x += dx;

        for (auto& ent : entities) {
            bool wasRiding = ent.onGround &&
                std::abs((ent.box.y + ent.box.h) - oldBox.y) <= eps &&
                (ent.box.x + ent.box.w > oldBox.x) && (ent.box.x  0.0f) 
                    ? (plat.box.x + plat.box.w) - ent.box.x 
                    : plat.box.x - (ent.box.x + ent.box.w);
                MoveEntityTilesOnly(ent, pushX, 0.0f, tileSize);

                if (AABBOverlap(plat.box, ent.box)) {
                    OnEntityCrushed(ent);
                }
            } else if (wasRiding) {
                // Carry rider horizontally through tilemap
                MoveEntityTilesOnly(ent, dx, 0.0f, tileSize);
            }
        }

        // 2. Move Y and check riders/pushes
        oldBox = plat.box;
        plat.box.y += dy;

        for (auto& ent : entities) {
            bool wasRiding = ent.onGround &&
                std::abs((ent.box.y + ent.box.h) - oldBox.y) <= eps &&
                (ent.box.x + ent.box.w > oldBox.x) && (ent.box.x  0.0f) 
                    ? (plat.box.y + plat.box.h) - ent.box.y 
                    : plat.box.y - (ent.box.y + ent.box.h);
                MoveEntityTilesOnly(ent, 0.0f, pushY, tileSize);

                if (AABBOverlap(plat.box, ent.box)) {
                    OnEntityCrushed(ent);
                }
            } else if (wasRiding && dy > 0.0f) {
                // Only carry downward explicitly; upward movement already pushes via overlap
                MoveEntityTilesOnly(ent, 0.0f, dy, tileSize);
            }
        }
    }
}
~~~~


Key Implementation Details

* Upward Y Movement: When a platform moves up (dy < 0), it naturally intersects the rider's feet and handles them via the push branch. Explicitly pulling the rider down when dy > 0 prevents the player from briefly losing onGround state and triggering fall animations every frame.
* Include Platforms in Entity Solver: During step 3 (normal entity movement), treat moving platforms as solid AABBs alongside static tiles so jumping into the underside of a platform or walking into its side resolves cleanly.
* Momentum Inheritance: When a player jumps off a moving platform, optionally add plat.vx (or a clamped fraction of it) to the player's horizontal velocity so jumps feel natural.
