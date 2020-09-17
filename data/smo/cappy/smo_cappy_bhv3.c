<<<<<<< HEAD
#include "smo_cappy_common.inl"
=======
#include "../smo_c_includes.h"

#define o gCurrentObject
>>>>>>> ed6bf96ae1f732967e9f72ea66c102467e719cb8

//
// Init
//

<<<<<<< HEAD
void cappy_bhv3_init(struct Object *cappy, struct MarioState *m) {
    cappy->oPosX = m->pos[0];
    cappy->oPosY = m->pos[1] + 30;
    cappy->oPosZ = m->pos[2];
    cappy->oFaceAngleYaw = m->faceAngle[1];
=======
static void bhv_cappy3_init() {
    o->oCappyRadius         = 240.f;
    o->oCappyRadiusGrowth   = 16.f;
    o->oCappyAngleVel       = 0x0C00;
    o->oCappyYaw0           = o->oMario->faceAngle[1];
    o->oCappyYaw            = 0;
    o->oCappyInvincible     = TRUE;
    o->oCappyInteractMario  = FALSE;
    o->oPosX                = o->oMario->pos[0];
    o->oPosY                = o->oMario->pos[1] + 30;
    o->oPosZ                = o->oMario->pos[2];
    o->oFaceAngleYaw        = 0;
    o->oFaceAnglePitch      = 0;
    o->oFaceAngleRoll       = 0;
    o->oCappyTimer          = 0;
>>>>>>> ed6bf96ae1f732967e9f72ea66c102467e719cb8
}

//
// Movement
//

#define STEPS 16
<<<<<<< HEAD
static void cappy_bhv3_move(struct Object *cappy, struct MarioState *m) {
    f32 radius = MIN(cappy->oTimer * 12.f, 180.f);
    s16 angle = (s16)(cappy->oFaceAngleYaw + (s32)(cappy->oTimer) * 0x0C00);
=======
static void cappy_fly_around_mario(struct Object *cappy, struct MarioState *m) {
    f32 radius = MIN(cappy->oCappyTimer * cappy->oCappyRadiusGrowth, cappy->oCappyRadius);
    s16 angle = (s16)(cappy->oCappyYaw0 + (s32)(cappy->oCappyTimer) * cappy->oCappyAngleVel);
>>>>>>> ed6bf96ae1f732967e9f72ea66c102467e719cb8
    Vec3f v = { radius * coss(angle), radius * sins(angle), 0 };
    vec3f_rotate_zxy(v, -m->faceAngle[0], m->faceAngle[1], 0);
    cappy->oPosX = m->pos[0];
    cappy->oPosY = m->pos[1] + 30;
    cappy->oPosZ = m->pos[2];
<<<<<<< HEAD
=======
    cappy->oCappyYaw = atan2s(v[2], v[0]);
>>>>>>> ed6bf96ae1f732967e9f72ea66c102467e719cb8

    for (s32 i = 0; i != STEPS; ++i) {
        f32 x = cappy->oPosX;
        f32 y = cappy->oPosY;
        f32 z = cappy->oPosZ;
        cappy->oPosX += (v[0] / STEPS);
        cappy->oPosY += (v[1] / STEPS);
        cappy->oPosZ += (v[2] / STEPS);

        // Walls
        struct WallCollisionData hitbox;
        hitbox.x = cappy->oPosX;
        hitbox.y = cappy->oPosY;
        hitbox.z = cappy->oPosZ;
<<<<<<< HEAD
        hitbox.offsetY = cappy->hitboxHeight / 2;
        hitbox.radius = cappy->oWallHitboxRadius;
        if (find_wall_collisions(&hitbox) != 0) {
            cappy_register_wall_object(cappy, m, hitbox.walls[0]->object);
=======
        hitbox.offsetY = CAPPY_HITBOX_HEIGHT / 2;
        hitbox.radius = CAPPY_WALL_RADIUS;
        if (find_wall_collisions(&hitbox) != 0) {
            cappy_register_wall_object(cappy, hitbox.walls[0]->object);
>>>>>>> ed6bf96ae1f732967e9f72ea66c102467e719cb8
            cappy->oPosX = hitbox.x;
            cappy->oPosY = hitbox.y;
            cappy->oPosZ = hitbox.z;
            break;
        }

        // Floor
        struct Surface *floor = NULL;
        f32 floorY = find_floor(cappy->oPosX, cappy->oPosY, cappy->oPosZ, &floor);
        if (floor != NULL) {
            f32 diffY = cappy->oPosY - floorY;
            if (diffY < 0) {
<<<<<<< HEAD
                cappy_register_floor_object(cappy, m, floor->object);
=======
                cappy_register_floor_object(cappy, floor->object);
>>>>>>> ed6bf96ae1f732967e9f72ea66c102467e719cb8
                if (floor->normal.y < 0.707f) { // ~45 deg
                    cappy->oPosX = x;
                    cappy->oPosY = y;
                    cappy->oPosZ = z;
                    break;
                } else {
                    cappy->oPosY = floorY;
                }
            }
        } else {
            cappy->oPosX = x;
            cappy->oPosY = y;
            cappy->oPosZ = z;
            break;
        }

        // Ceiling
        struct Surface *ceil = NULL;
        f32 ceilY = find_ceil(cappy->oPosX, cappy->oPosY, cappy->oPosZ, &ceil);
        if (ceil != NULL) {
<<<<<<< HEAD
            f32 h = (cappy->hitboxHeight - cappy->hitboxDownOffset);
            if (cappy->oPosY <= ceilY && cappy->oPosY + h > ceilY) {
                cappy_register_ceiling_object(cappy, m, ceil->object);
=======
            f32 h = (CAPPY_HITBOX_HEIGHT - CAPPY_HITBOX_DOWNOFFSET);
            if (cappy->oPosY <= ceilY && cappy->oPosY + h > ceilY) {
                cappy_register_ceiling_object(cappy, ceil->object);
>>>>>>> ed6bf96ae1f732967e9f72ea66c102467e719cb8
                if (ceil->normal.y > -0.707f) { // ~45 deg
                    cappy->oPosX = x;
                    cappy->oPosY = y;
                    cappy->oPosZ = z;
                    break;
                } else {
                    cappy->oPosY = MAX(ceilY - h, floorY);
                }
            }
        }
    }
}
#undef STEPS

//
<<<<<<< HEAD
// Update
//

void cappy_bhv3_update(struct Object *cappy, struct MarioState *m) {
    m->oCappyInteractMario = FALSE;
    if (m->action != ACT_FLYING) {
        cappy_start_return_to_mario(cappy);
    } else {
        if (cappy->oTimer < 12) {
            m->marioObj->header.gfx.angle[2] += ((cappy->oTimer * 65536) / 12);
        }
        cappy_bhv3_move(cappy, m);
    }
}
=======
// Loop
//

static void bhv_cappy3_loop(void) {
    if (o->oCappyTimer < CAPPY_MAX_LIFETIME) {
        o->oCappyInteractMario = FALSE;
        if (o->oMario->action != ACT_FLYING) {
            cappy_start_return_to_mario(o);
        } else {
            if (o->oCappyTimer < 12) {
                o->oMarioObj->header.gfx.angle[2] += ((o->oCappyTimer * 65536) / 12);
            }
            cappy_fly_around_mario(o, o->oMario);
            cappy_process_object_collision(o, o->oMario);
        }
    } else {
        if (cappy_return_to_mario(o, o->oMario)) {
            smo_obj_free_data(o);
            obj_mark_for_deletion(o);
            return;
        }
    }

    spawn_object(o, MODEL_NONE, bhvSparkleSpawn);
    o->oFaceAngleYaw += 0x2000;
    o->oOpacity = ((o->oMario->flags & MARIO_VANISH_CAP) ? 0x80 : 0xFF);
    o->oCappyTimer++;
}

#undef o

//
// Behavior
//

const BehaviorScript bhvCappy3[] = {
    0x00060000,
    0x11010041,
    0x0C000000, (uintptr_t)(bhv_cappy3_init),
    0x08000000,
    0x0C000000, (uintptr_t)(bhv_cappy3_loop),
    0x09000000
};



>>>>>>> ed6bf96ae1f732967e9f72ea66c102467e719cb8