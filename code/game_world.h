#if !defined(GAME_WORLD_H)
#define GAME_WORLD_H

#include "game.h"

#define TILE_WIDTH 2.0f
#define TILE_HEIGHT 2.0f

#define ROW_TILE_COUNT		10
#define COLUMN_TILE_COUNT	10

//#define METER_TO_PIXELS	5.0f

uint8 World[ROW_TILE_COUNT][COLUMN_TILE_COUNT] = {
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
	{1, 1, 0, 0, 0, 0, 0, 1, 1, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 1, 1, 0, 0, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};


uint8 GetTile(int r, int c)
{
	return World[r][c];
}

float TileToWorldX(int col)
{
	return col * TILE_WIDTH;
}

float TileToWorldY(int row)
{
	return row * TILE_HEIGHT;
}

int WorldToTileCol(float worldX)
{
	int Result = FloorReal32ToInt32(worldX / TILE_WIDTH);
	return Result;
}

int WorldToTileRow(float worldY)
{
	int Result = FloorReal32ToInt32(worldY / TILE_HEIGHT);
	return Result;
}

float TileLeft(int tx)
{
	return (tx * TILE_WIDTH);
}

float TileRight(int tx)
{
	return ((tx + 1) * TILE_WIDTH);
}

float TileBottom(int ty)
{
	return ((ty + 1) * TILE_HEIGHT);
}

float TileTop(int ty)
{
	return (ty * TILE_HEIGHT);
}

// Small epsilon keeps the AABB from falsely overlapping adjacent rows/columns
const float EPS = 0.001f;

void ResolveX(hero_entity* e, float dx) {
    if (dx == 0.0f) return;
    e->box.x += dx;

    int minTx = WorldToTileCol(e->box.x - e->box.halfW);			//Ensure WorldToTile uses std::floor(coord / tileSize)
    int maxTx = WorldToTileCol(e->box.x + e->box.halfW);
    int minTy = WorldToTileRow(e->box.y - e->box.halfH + EPS);
    int maxTy = WorldToTileRow(e->box.y + e->box.halfH - EPS);

    for (int ty = minTy; ty <= maxTy; ++ty) {
        for (int tx = minTx; tx <= maxTx; ++tx) {
            if (GetTile(ty, tx) != 1) continue;

            if (dx > 0.0f) { // Moving right -> snap to tile left edge
                e->box.x = TileLeft(tx) - e->box.halfW;
            } else {         // Moving left -> snap to tile right edge
                e->box.x = TileRight(tx) + e->box.halfW;
            }
            //e.vx = 0.0f;
            return;
        }
    }
}

void ResolveY(hero_entity* e, float dy) {
    if (dy == 0.0f) return;
    float prevBottom = e->box.y + e->box.halfH; // Assuming +Y is down
    e->box.y += dy;

    int minTx = WorldToTileCol(e->box.x - e->box.halfW + EPS);
    int maxTx = WorldToTileCol(e->box.x + e->box.halfW - EPS);
    int minTy = WorldToTileRow(e->box.y - e->box.halfH);
    int maxTy = WorldToTileRow(e->box.y + e->box.halfH);

    for (int ty = minTy; ty <= maxTy; ++ty) {
        for (int tx = minTx; tx <= maxTx; ++tx) {
            int type = GetTile(ty, tx);
            if (type == 0) continue;

            // One-way platform: only collide when falling and previously above the top edge
            if (type == 2) {
                float tileTop = TileTop(ty);
                if (dy <= 0.0f || prevBottom > tileTop + EPS) continue;
            }

            if (dy > 0.0f) { // Falling -> land on tile top
                e->box.y = TileTop(ty) - e->box.halfH;
                e->onGround = true;
            } else {         // Jumping -> bonk tile bottom
                e->box.y = TileBottom(ty) + e->box.halfH;
            }
            //e.vy = 0.0f;
            return;
        }
    }
}

void MoveEntity(hero_entity* e, float totalDx, float totalDy)
{	
	// Step size <= half the smallest dimension to prevent tunneling
    float maxStep = min_float(e->box.halfW, e->box.halfH) * 0.5f;
    float maxDelta = max_float(fabsf(totalDx), fabsf(totalDy));
    int steps = max_int(1, (int)(CeilReal32ToInt32(maxDelta / maxStep)));

    float stepDx = totalDx / steps;
    float stepDy = totalDy / steps;

    //e.onGround = false;

    for (int i = 0; i < steps; ++i) {
        ResolveX(e, stepDx);
        ResolveY(e, stepDy);
    }
}

#endif

