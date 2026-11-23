## A C++ implementation of Swept AABB with velocity sliding and internal-edge filtering

Here is a self-contained C++17 implementation of Swept AABB using Minkowski sum expansion, iterative velocity sliding, and tilemap internal-edge filtering.


Key Design Mechanics
------------------------------

* Minkowski Sum Expansion: Instead of sweeping a box against a box, each solid tile's AABB is expanded by the entity's half-extents, reducing the problem to a raycast from the entity's center.
* Internal-Edge Filtering: Before accepting a collision normal (e.g., (-1, 0)), the solver checks if the adjacent tile in that normal's direction is also solid. If it is, that edge is an internal seam and is ignored so the entity never snags while sliding across flat floors or walls.
* Skin Epsilon (SKIN): Stops the entity a tiny fraction (1e-4f) before the exact surface so floating-point rounding never leaves it embedded inside a tile on the next iteration.

~~~~ cpp
#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(Vec2 r) const { return {x + r.x, y + r.y}; }
    Vec2 operator-(Vec2 r) const { return {x - r.x, y - r.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};

inline float Dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

struct AABB {
    Vec2 center;
    Vec2 halfSize;
};

struct RayHit {
    bool hit = false;
    float t = 1.0f;      // Time of impact in [0, 1]
    Vec2 normal = {0, 0};
};

// Simple grid-based tilemap interface
struct Tilemap {
    int width = 0;
    int height = 0;
    float tileSize = 16.0f;
    std::vector<uint8_t> solid; // 1 = solid, 0 = empty

    bool IsSolid(int tx, int ty) const {
        if (tx < 0 || ty < 0 || tx >= width || ty >= height) return false;
        return solid[ty * width + tx] != 0;
    }

    AABB GetTileAABB(int tx, int ty) const {
        float half = tileSize * 0.5f;
        return {
            {tx * tileSize + half, ty * tileSize + half},
            {half, half}
        };
    }
};

// Ray vs. Static AABB (Slab Method)
// Ray starts at `origin` and travels along `delta` (velocity * dt).
RayHit SweepRayVsAABB(Vec2 origin, Vec2 delta, const AABB& target) {
    RayHit result;
    const float EPS = 1e-6f;

    float tEntryX, tExitX;
    if (std::abs(delta.x) < EPS) {
        if (origin.x < target.center.x - target.halfSize.x ||
            origin.x > target.center.x + target.halfSize.x) {
            return result;
        }
        tEntryX = -std::numeric_limits<float>::infinity();
        tExitX  =  std::numeric_limits<float>::infinity();
    } else {
        float invDx = 1.0f / delta.x;
        float t1 = (target.center.x - target.halfSize.x - origin.x) * invDx;
        float t2 = (target.center.x + target.halfSize.x - origin.x) * invDx;
        tEntryX = std::min(t1, t2);
        tExitX  = std::max(t1, t2);
    }

    float tEntryY, tExitY;
    if (std::abs(delta.y) < EPS) {
        if (origin.y < target.center.y - target.halfSize.y ||
            origin.y > target.center.y + target.halfSize.y) {
            return result;
        }
        tEntryY = -std::numeric_limits<float>::infinity();
        tExitY  =  std::numeric_limits<float>::infinity();
    } else {
        float invDy = 1.0f / delta.y;
        float t1 = (target.center.y - target.halfSize.y - origin.y) * invDy;
        float t2 = (target.center.y + target.halfSize.y - origin.y) * invDy;
        tEntryY = std::min(t1, t2);
        tExitY  = std::max(t1, t2);
    }

    float tEntry = std::max(tEntryX, tEntryY);
    float tExit  = std::min(tExitX, tExitY);

    // Reject if no overlap, or if box is behind ray, or already overlapping (tEntry < 0)
    if (tEntry > tExit || tExit < 0.0f || tEntry < 0.0f || tEntry > 1.0f) {
        return result;
    }

    result.hit = true;
    result.t = tEntry;

    if (tEntryX > tEntryY) {
        result.normal = {delta.x > 0.0f ? -1.0f : 1.0f, 0.0f};
    } else {
        result.normal = {0.0f, delta.y > 0.0f ? -1.0f : 1.0f};
    }

    return result;
}

struct MoveResult {
    Vec2 finalPosition;
    Vec2 finalVelocity;
    bool onGround = false;
    bool hitWall = false;
    bool hitCeiling = false;
};

// Moves an entity through the tilemap with continuous sweep and sliding
MoveResult MoveAndSlide(AABB entity, Vec2 velocity, float dt, const Tilemap& map) {
    MoveResult out;
    Vec2 remainingMove = velocity * dt;
    const int MAX_SLIDE_ITERATIONS = 3;
    const float SKIN = 1e-4f;

    for (int iter = 0; iter (std::floor(minX / map.tileSize));
        int tileMaxX = static_cast<int>(std::floor(maxX / map.tileSize));
        int tileMinY = static_cast<int>(std::floor(minY / map.tileSize));
        int tileMaxY = static_cast<int>(std::floor(maxY / map.tileSize));

        RayHit earliestHit;
        earliestHit.t = 1.0f;

        // 2. Narrowphase: test swept ray against Minkowski-expanded tiles
        for (int ty = tileMinY; ty (hit.normal.x);
                int neighborY = ty + static_cast<int>(hit.normal.y);
                if (map.IsSolid(neighborX, neighborY)) {
                    continue;
                }

                if (hit.t  0.0f) ? (SKIN / moveLen) : 0.0f;
        float safeT = std::max(0.0f, earliestHit.t - skinT);

        entity.center = entity.center + remainingMove * safeT;

        // Track contact flags (assumes +Y is up; flip if +Y is down)
        if (earliestHit.normal.y > 0.5f)  out.onGround = true;
        if (earliestHit.normal.y < -0.5f) out.hitCeiling = true;
        if (std::abs(earliestHit.normal.x) > 0.5f) out.hitWall = true;

        // 5. Slide remaining displacement and velocity along the hit surface
        float leftoverRatio = 1.0f - safeT;
        Vec2 leftoverMove = remainingMove * leftoverRatio;

        remainingMove = leftoverMove - earliestHit.normal * Dot(leftoverMove, earliestHit.normal);
        velocity      = velocity     - earliestHit.normal * Dot(velocity, earliestHit.normal);
    }

    out.finalPosition = entity.center;
    out.finalVelocity = velocity;
    return out;
}
~~~~


Important Implementation Details
-----------------------------------------------

* Coordinate Orientation: The contact flags (onGround, hitCeiling) assume +Y points up. If your engine uses screen coordinates where +Y points down, swap normal.y > 0.5f and normal.y < -0.5f.
* Initial Overlap Recovery: SweepRayVsAABB intentionally rejects rays starting inside an expanded box (tEntry < 0.0f) so parallel sliding contacts don't re-trigger at t = 0. If an entity ever gets spawned or pushed inside a tile by a moving platform, run a quick discrete Minkowski push-out pass before calling MoveAndSlide.
