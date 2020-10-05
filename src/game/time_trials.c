#include "time_trials.h"
#include "sm64.h"
#include "game_init.h"
#include "geo_commands.h"
#include "gfx_dimensions.h"
#include "ingame_menu.h"
#include "level_update.h"
#include "object_helpers.h"
#include "object_list_processor.h"
#include "save_file.h"
#include "segment2.h"
#include "spawn_object.h"
#include "pc/cheats.h"
#include "pc/cliopts.h"
#include "pc/configfile.h"
#include "pc/fs/fs.h"
#include "engine/math_util.h"
#include <stdio.h>

#ifdef VERSION_EU
#include "eu_translation.h"
extern s32 gInGameLanguage;
#define seg2_course_name_table                                                                         \
    (gInGameLanguage == LANGUAGE_GERMAN                                                                \
         ? course_name_table_eu_de                                                                     \
         : (gInGameLanguage == LANGUAGE_FRENCH ? course_name_table_eu_fr : course_name_table_eu_en))
#endif

//
// Models
//

#include "time_trials_ghost_geo.inl"

static void time_trials_load_models() {
    struct AllocOnlyPool *pool = alloc_only_pool_init(main_pool_available() - sizeof(struct AllocOnlyPool), MEMORY_POOL_LEFT);
    gLoadedGraphNodes[MODEL_COSMIC_MARIO] = process_geo_layout(pool, (void *) cosmic_mario_geo);
    alloc_only_pool_resize(pool, pool->usedSpace);
}

//
// Utils
//

static const char *time_trials_get_save_filename() {
    static char buffer[256];
    snprintf(buffer, 256, "%s", SAVE_FILENAME);
    buffer[strlen(buffer) - 4] = 0; // cut extension
    return buffer;
}

static const char *time_trials_get_folder(const char *path) {
    static char buffer[256];
    snprintf(buffer, 256, "%s/%s", path, time_trials_get_save_filename());
    return buffer;
}

static const char *time_trials_get_filename(const char *path, s32 fileIndex, s32 slotIndex) {
    static char buffer[256];
    snprintf(buffer, 256, "%s/%d.%d.ttg", time_trials_get_folder(path), fileIndex, slotIndex);
    return buffer;
}

static const u8 *time_trials_int_to_sm64_string(s32 value, const char *format) {
    static u8 buffer[64];
    bzero(buffer, 64);
    if (!format || strlen(format) > 8) {
        return buffer;
    }
    s32 len = snprintf((char *) buffer, 64, format, value);
    for (s32 i = 0; i < len; ++i) {
        if (buffer[i] >= 'A' && buffer[i] <= 'F') {
            buffer[i] = buffer[i] - 'A';
        } else if (buffer[i] >= 'a' && buffer[i] <= 'f') {
            buffer[i] = buffer[i] - 'a';
        } else {
            buffer[i] = buffer[i] - '0';
        }
    }
    buffer[len] = DIALOG_CHAR_TERMINATOR;
    return buffer;
}

//
// Time Trials Slots
//

#define NUM_SLOTS                   sizeof(sTimeTrialSlots) / sizeof(sTimeTrialSlots[0])
#define STAR_CODE(course, star)     (u16)(((u16)(course & 0xFF) << 8) | ((u16)(star & 0xFF) << 0))

static const u16 sTimeTrialSlots[] = {

    /* Main courses */
    STAR_CODE(COURSE_BOB, 0),
    STAR_CODE(COURSE_BOB, 1),
    STAR_CODE(COURSE_BOB, 2),
    STAR_CODE(COURSE_BOB, 3),
    STAR_CODE(COURSE_BOB, 4),
    STAR_CODE(COURSE_BOB, 5),
    STAR_CODE(COURSE_BOB, 6),
    STAR_CODE(COURSE_WF, 0),
    STAR_CODE(COURSE_WF, 1),
    STAR_CODE(COURSE_WF, 2),
    STAR_CODE(COURSE_WF, 3),
    STAR_CODE(COURSE_WF, 4),
    STAR_CODE(COURSE_WF, 5),
    STAR_CODE(COURSE_WF, 6),
    STAR_CODE(COURSE_JRB, 0),
    STAR_CODE(COURSE_JRB, 1),
    STAR_CODE(COURSE_JRB, 2),
    STAR_CODE(COURSE_JRB, 3),
    STAR_CODE(COURSE_JRB, 4),
    STAR_CODE(COURSE_JRB, 5),
    STAR_CODE(COURSE_JRB, 6),
    STAR_CODE(COURSE_CCM, 0),
    STAR_CODE(COURSE_CCM, 1),
    STAR_CODE(COURSE_CCM, 2),
    STAR_CODE(COURSE_CCM, 3),
    STAR_CODE(COURSE_CCM, 4),
    STAR_CODE(COURSE_CCM, 5),
    STAR_CODE(COURSE_CCM, 6),
    STAR_CODE(COURSE_BBH, 0),
    STAR_CODE(COURSE_BBH, 1),
    STAR_CODE(COURSE_BBH, 2),
    STAR_CODE(COURSE_BBH, 3),
    STAR_CODE(COURSE_BBH, 4),
    STAR_CODE(COURSE_BBH, 5),
    STAR_CODE(COURSE_BBH, 6),
    STAR_CODE(COURSE_HMC, 0),
    STAR_CODE(COURSE_HMC, 1),
    STAR_CODE(COURSE_HMC, 2),
    STAR_CODE(COURSE_HMC, 3),
    STAR_CODE(COURSE_HMC, 4),
    STAR_CODE(COURSE_HMC, 5),
    STAR_CODE(COURSE_HMC, 6),
    STAR_CODE(COURSE_LLL, 0),
    STAR_CODE(COURSE_LLL, 1),
    STAR_CODE(COURSE_LLL, 2),
    STAR_CODE(COURSE_LLL, 3),
    STAR_CODE(COURSE_LLL, 4),
    STAR_CODE(COURSE_LLL, 5),
    STAR_CODE(COURSE_LLL, 6),
    STAR_CODE(COURSE_SSL, 0),
    STAR_CODE(COURSE_SSL, 1),
    STAR_CODE(COURSE_SSL, 2),
    STAR_CODE(COURSE_SSL, 3),
    STAR_CODE(COURSE_SSL, 4),
    STAR_CODE(COURSE_SSL, 5),
    STAR_CODE(COURSE_SSL, 6),
    STAR_CODE(COURSE_DDD, 0),
    STAR_CODE(COURSE_DDD, 1),
    STAR_CODE(COURSE_DDD, 2),
    STAR_CODE(COURSE_DDD, 3),
    STAR_CODE(COURSE_DDD, 4),
    STAR_CODE(COURSE_DDD, 5),
    STAR_CODE(COURSE_DDD, 6),
    STAR_CODE(COURSE_SL, 0),
    STAR_CODE(COURSE_SL, 1),
    STAR_CODE(COURSE_SL, 2),
    STAR_CODE(COURSE_SL, 3),
    STAR_CODE(COURSE_SL, 4),
    STAR_CODE(COURSE_SL, 5),
    STAR_CODE(COURSE_SL, 6),
    STAR_CODE(COURSE_WDW, 0),
    STAR_CODE(COURSE_WDW, 1),
    STAR_CODE(COURSE_WDW, 2),
    STAR_CODE(COURSE_WDW, 3),
    STAR_CODE(COURSE_WDW, 4),
    STAR_CODE(COURSE_WDW, 5),
    STAR_CODE(COURSE_WDW, 6),
    STAR_CODE(COURSE_TTM, 0),
    STAR_CODE(COURSE_TTM, 1),
    STAR_CODE(COURSE_TTM, 2),
    STAR_CODE(COURSE_TTM, 3),
    STAR_CODE(COURSE_TTM, 4),
    STAR_CODE(COURSE_TTM, 5),
    STAR_CODE(COURSE_TTM, 6),
    STAR_CODE(COURSE_THI, 0),
    STAR_CODE(COURSE_THI, 1),
    STAR_CODE(COURSE_THI, 2),
    STAR_CODE(COURSE_THI, 3),
    STAR_CODE(COURSE_THI, 4),
    STAR_CODE(COURSE_THI, 5),
    STAR_CODE(COURSE_THI, 6),
    STAR_CODE(COURSE_TTC, 0),
    STAR_CODE(COURSE_TTC, 1),
    STAR_CODE(COURSE_TTC, 2),
    STAR_CODE(COURSE_TTC, 3),
    STAR_CODE(COURSE_TTC, 4),
    STAR_CODE(COURSE_TTC, 5),
    STAR_CODE(COURSE_TTC, 6),
    STAR_CODE(COURSE_RR, 0),
    STAR_CODE(COURSE_RR, 1),
    STAR_CODE(COURSE_RR, 2),
    STAR_CODE(COURSE_RR, 3),
    STAR_CODE(COURSE_RR, 4),
    STAR_CODE(COURSE_RR, 5),
    STAR_CODE(COURSE_RR, 6),

    /* Bowser Courses */
    STAR_CODE(COURSE_BITDW, 0),
    STAR_CODE(COURSE_BITDW, 1),
    STAR_CODE(COURSE_BITFS, 0),
    STAR_CODE(COURSE_BITFS, 1),
    STAR_CODE(COURSE_BITS, 0),
    STAR_CODE(COURSE_BITS, 1),

    /* Secret Courses */
    STAR_CODE(COURSE_PSS, 0),
    STAR_CODE(COURSE_PSS, 1),
    STAR_CODE(COURSE_SA, 0),
    STAR_CODE(COURSE_WMOTR, 0),
    STAR_CODE(COURSE_TOTWC, 0),
    STAR_CODE(COURSE_VCUTM, 0),
    STAR_CODE(COURSE_COTMC, 0),
};

static s32 time_trials_get_slot_index(s32 course, s32 star) {
    u16 starCode = STAR_CODE(course, star);
    for (u32 i = 0; i != NUM_SLOTS; ++i) {
        if (sTimeTrialSlots[i] == starCode) {
            return (s32) i;
        }
    }
    return -1;
}

#undef STAR_CODE

//
// Ghost Frame data
//

struct TimeTrialGhostFrameData {
    s16 posX;
    s16 posY;
    s16 posZ;
    s8 pitch;
    s8 yaw;
    s8 roll;
    u8 scaleX;
    u8 scaleY;
    u8 scaleZ;
    u8 animIndex;
    u8 animFrame;
    u8 level;
    u8 area;
};

//
// Data
//

enum { TT_TIMER_DISABLED, TT_TIMER_RUNNING, TT_TIMER_STOPPED };
static s8 sTimeTrialTimerState = TT_TIMER_DISABLED;
static s16 sTimeTrialTimer = 0;
static s8 sTimeTrialHiScore = FALSE;
static s16 sTimeTrialTimes[NUM_SAVE_FILES][NUM_SLOTS];
static struct TimeTrialGhostFrameData sTimeTrialGhostData[TIME_TRIALS_MAX_ALLOWED_TIME];
static s16 sStartingMarioHealth = 0;

//
// Read
//

static u16 read_u16(FILE *f) {
    u8 low, high;
    if (fread(&high, 1, 1, f)) {
        if (fread(&low, 1, 1, f)) {
            return (u16)(((u16)(high) << 8) | ((u16)(low)));
        }
    }
    return 0;
}

static u8 read_u8(FILE *f) {
    u8 byte;
    if (fread(&byte, 1, 1, f)) {
        return byte;
    }
    return 0;
}

static s16 time_trials_read_ghost_data(s32 fileIndex, s32 slot, s8 readLengthOnly) {
    bzero(sTimeTrialGhostData, sizeof(sTimeTrialGhostData));

    // Valid slot
    if (slot == -1) {
        return 0;
    }

    // Open file
    const char *filename = time_trials_get_filename(fs_writepath, fileIndex, slot);
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        return 0;
    }

    // Read length
    s16 length = (s16) read_u16(f);
    if (readLengthOnly) {
        fclose(f);
        return length;
    }

    // Read data
    for (s16 i = 0; i != length; ++i) {
        sTimeTrialGhostData[i].posX      = (s16) read_u16(f);
        sTimeTrialGhostData[i].posY      = (s16) read_u16(f);
        sTimeTrialGhostData[i].posZ      = (s16) read_u16(f);
        sTimeTrialGhostData[i].pitch     = (s8) read_u8(f);
        sTimeTrialGhostData[i].yaw       = (s8) read_u8(f);
        sTimeTrialGhostData[i].roll      = (s8) read_u8(f);
        sTimeTrialGhostData[i].scaleX    = read_u8(f);
        sTimeTrialGhostData[i].scaleY    = read_u8(f);
        sTimeTrialGhostData[i].scaleZ    = read_u8(f);
        sTimeTrialGhostData[i].animIndex = read_u8(f);
        sTimeTrialGhostData[i].animFrame = read_u8(f);
        sTimeTrialGhostData[i].level     = read_u8(f);
        sTimeTrialGhostData[i].area      = read_u8(f);
    }

    fclose(f);
    return length;
}

void time_trials_init_times() {
    for (s32 fileIndex = 0; fileIndex != NUM_SAVE_FILES; ++fileIndex) {
        for (u32 slot = 0; slot != NUM_SLOTS; ++slot) {
            s16 t = time_trials_read_ghost_data(fileIndex, slot, TRUE);
            if (t <= 0 || t > TIME_TRIALS_MAX_ALLOWED_TIME) {
                t = TIME_TRIALS_UNDEFINED_TIME;
            }
            sTimeTrialTimes[fileIndex][slot] = t;
        }
    }
}

//
// Write
//

static void write_u16(FILE *f, u16 word) {
    u8 low = (u8)(word);
    u8 high = (u8)(word >> 8);
    fwrite(&high, 1, 1, f);
    fwrite(&low, 1, 1, f);
}

static void write_u8(FILE *f, u8 byte) {
    fwrite(&byte, 1, 1, f);
}

static void time_trials_write_ghost_data(s32 fileIndex, s32 slot) {

    // Make folder
    fs_sys_mkdir(time_trials_get_folder(fs_writepath));

    // Open file
    const char *filename = time_trials_get_filename(fs_writepath, fileIndex, slot);
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        return;
    }

    // Write length
    s16 length = sTimeTrialTimer;
    write_u16(f, length);

    // Write data
    for (s16 i = 0; i != length; ++i) {
        write_u16(f, (u16) sTimeTrialGhostData[i].posX);
        write_u16(f, (u16) sTimeTrialGhostData[i].posY);
        write_u16(f, (u16) sTimeTrialGhostData[i].posZ);
        write_u8(f, (u8) sTimeTrialGhostData[i].pitch);
        write_u8(f, (u8) sTimeTrialGhostData[i].yaw);
        write_u8(f, (u8) sTimeTrialGhostData[i].roll);
        write_u8(f, sTimeTrialGhostData[i].scaleX);
        write_u8(f, sTimeTrialGhostData[i].scaleY);
        write_u8(f, sTimeTrialGhostData[i].scaleZ);
        write_u8(f, sTimeTrialGhostData[i].animIndex);
        write_u8(f, sTimeTrialGhostData[i].animFrame);
        write_u8(f, sTimeTrialGhostData[i].level);
        write_u8(f, sTimeTrialGhostData[i].area);
    }

    // Update times
    sTimeTrialTimes[fileIndex][slot] = length;
    fclose(f);
}

void time_trials_save_time(s32 fileIndex, s32 course, s32 level, s32 star, s32 noExit) {
    if (sTimeTrialTimerState == TT_TIMER_RUNNING) {

        // Bowser Key or Grand Star
        if ((!noExit) && (level == LEVEL_BOWSER_1 || level == LEVEL_BOWSER_2 || level == LEVEL_BOWSER_3)) {
            star = 1;
        }

        // Write time and ghost data if new record
        s32 slot = time_trials_get_slot_index(course, star);
        if (slot != -1) {
            s16 t = sTimeTrialTimes[fileIndex][slot];
            if (t == TIME_TRIALS_UNDEFINED_TIME || t > sTimeTrialTimer) {
                time_trials_write_ghost_data(fileIndex, slot);
                sTimeTrialHiScore = TRUE;
            }
        }
    }
}

//
// Get
//

s16 time_trials_get_time(s32 fileIndex, s32 course, s32 star) {
    s32 slot = time_trials_get_slot_index(course, star);
    if (slot == -1) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    return sTimeTrialTimes[fileIndex][slot];
}

s32 time_trials_get_course_time(s32 fileIndex, s32 course) {
    s32 courseTime = 0;
    for (s32 star = 0;; star++) {
        s32 slot = time_trials_get_slot_index(course, star);
        if (slot == -1) {
            break;
        }

        s16 t = sTimeTrialTimes[fileIndex][slot];
        if (t == TIME_TRIALS_UNDEFINED_TIME) {
            return TIME_TRIALS_UNDEFINED_TIME;
        }

        courseTime += (s32) t;
    }
    return courseTime;
}

s32 time_trials_get_bowser_time(s32 fileIndex) {
    s32 bitdw = time_trials_get_course_time(fileIndex, COURSE_BITDW);
    if (bitdw == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 bitfs = time_trials_get_course_time(fileIndex, COURSE_BITFS);
    if (bitfs == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 bits = time_trials_get_course_time(fileIndex, COURSE_BITS);
    if (bits == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    return bitdw + bitfs + bits;
}

s32 time_trials_get_secret_time(s32 fileIndex) {
    s32 pss = time_trials_get_course_time(fileIndex, COURSE_PSS);
    if (pss == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 cotmc = time_trials_get_course_time(fileIndex, COURSE_COTMC);
    if (cotmc == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 totwc = time_trials_get_course_time(fileIndex, COURSE_TOTWC);
    if (totwc == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 vcutm = time_trials_get_course_time(fileIndex, COURSE_VCUTM);
    if (vcutm == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 wmotr = time_trials_get_course_time(fileIndex, COURSE_WMOTR);
    if (wmotr == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    s32 sa = time_trials_get_course_time(fileIndex, COURSE_SA);
    if (sa == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }
    return pss + cotmc + totwc + vcutm + wmotr + sa;
}

s32 time_trials_get_total_time(s32 fileIndex) {
    s32 coursesTime = 0;
    for (s32 course = COURSE_MIN; course <= COURSE_STAGES_MAX; course++) {
        s32 t = time_trials_get_course_time(fileIndex, course);
        if (t == TIME_TRIALS_UNDEFINED_TIME) {
            return TIME_TRIALS_UNDEFINED_TIME;
        }
        coursesTime += t;
    }

    s32 bowserTime = time_trials_get_bowser_time(fileIndex);
    if (bowserTime == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }

    s32 secretTime = time_trials_get_secret_time(fileIndex);
    if (secretTime == TIME_TRIALS_UNDEFINED_TIME) {
        return TIME_TRIALS_UNDEFINED_TIME;
    }

    return coursesTime + bowserTime + secretTime;
}

//
// Ghost
//

static const BehaviorScript bhvTimeTrialGhost[] = {
    0x08000000,
    0x09000000
};

struct TimeTrialGhostAnimationData {
    struct Animation animation;
    u8 animationData[0x40000];
};
static struct TimeTrialGhostAnimationData sGhostAnimData;

static u16 time_trials_ghost_retrieve_current_animation_index(struct MarioState *m) {
    struct MarioAnimDmaRelatedThing *animDmaTable = m->animation->animDmaTable;
    void *a = m->animation->currentAnimAddr;
    for (u16 i = 0; i != animDmaTable->count; ++i) {
        void *b = animDmaTable->srcAddr + animDmaTable->anim[i].offset;
        if (a == b) {
            return i;
        }
    }
    return 0;
}

static s32 time_trials_ghost_load_animation(struct MarioState *m, u16 animIndex) {
    struct MarioAnimDmaRelatedThing *animDmaTable = m->animation->animDmaTable;
    if (animIndex >= animDmaTable->count) {
        return FALSE;
    }

    // Raw data
    u8 *addr = animDmaTable->srcAddr + animDmaTable->anim[animIndex].offset;
    u32 size = animDmaTable->anim[animIndex].size;        
    bcopy(addr, sGhostAnimData.animationData, size);

    // Seek index and values pointers
    struct Animation *a = (struct Animation *) sGhostAnimData.animationData;
    const u16 *indexStart = (const u16 *) ((uintptr_t)(sGhostAnimData.animationData) + (uintptr_t)(a->index));
    const s16 *valueStart = (const s16 *) ((uintptr_t)(sGhostAnimData.animationData) + (uintptr_t)(a->values));

    // Fill ghost animation data
    bcopy(sGhostAnimData.animationData, &sGhostAnimData.animation, sizeof(struct Animation));
    sGhostAnimData.animation.index = indexStart;
    sGhostAnimData.animation.values = valueStart;
    return TRUE;
}

static void time_trials_ghost_update_animation(struct Object *obj, struct MarioState *m, u16 animIndex, u16 animFrame) {
    static u16 sPreviousValidAnimIndex = 0xFFFF;
    if (obj->header.gfx.unk38.curAnim == NULL) {
        sPreviousValidAnimIndex = 0xFFFF;
    }

    // Load & set animation
    if (animIndex != sPreviousValidAnimIndex && time_trials_ghost_load_animation(m, animIndex)) {
        obj->header.gfx.unk38.curAnim = &sGhostAnimData.animation;
        obj->header.gfx.unk38.animAccel = 0;
        obj->header.gfx.unk38.animYTrans = m->unkB0;
        sPreviousValidAnimIndex = animIndex;
    }

    // Set frame
    if (obj->header.gfx.unk38.curAnim != NULL) {
        obj->header.gfx.unk38.animFrame = MIN(animFrame, obj->header.gfx.unk38.curAnim->unk08 - 1);
    }
}

static struct Object *obj_get_time_trials_ghost() {
    struct ObjectNode *listHead = &gObjectLists[OBJ_LIST_DEFAULT];
    struct Object *next = (struct Object *) listHead->next;
    while (next != (struct Object *) listHead) {
        if (next->behavior == bhvTimeTrialGhost && next->activeFlags != 0) {
            return next;
        }
        next = (struct Object *) next->header.next;
    }
    return NULL;
}

static void time_trials_update_ghost(struct MarioState *m, s16 frame, s16 level, s16 area) {
    struct Object *ghost = obj_get_time_trials_ghost();

    // If timer reached its max or frame data is ended, unload the ghost
    if (frame >= TIME_TRIALS_MAX_ALLOWED_TIME || !sTimeTrialGhostData[frame].level) {
        if (ghost != NULL) {
            obj_mark_for_deletion(ghost);
        }
        return;
    }

    // Spawn ghost if not loaded
    if (ghost == NULL) {
        ghost = spawn_object(m->marioObj, MODEL_COSMIC_MARIO, bhvTimeTrialGhost);
    }

    // Hide ghost if disabled or its level or area differs from Mario
    if (TIME_TRIALS_GHOST == 0 ||
        sTimeTrialGhostData[frame].level != (u8) level ||
        sTimeTrialGhostData[frame].area != (u8) area) {
        ghost->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
        return;
    }

    // Update ghost
    ghost->hitboxRadius         = 0;
    ghost->hitboxHeight         = 0;
    ghost->oOpacity             = 200;
    ghost->oPosX                = (f32) sTimeTrialGhostData[frame].posX;
    ghost->oPosY                = (f32) sTimeTrialGhostData[frame].posY;
    ghost->oPosZ                = (f32) sTimeTrialGhostData[frame].posZ;
    ghost->oFaceAnglePitch      = (s16) sTimeTrialGhostData[frame].pitch * 0x100;
    ghost->oFaceAngleYaw        = (s16) sTimeTrialGhostData[frame].yaw * 0x100;
    ghost->oFaceAngleRoll       = (s16) sTimeTrialGhostData[frame].roll * 0x100;
    ghost->oMoveAnglePitch      = (s16) sTimeTrialGhostData[frame].pitch * 0x100;
    ghost->oMoveAngleYaw        = (s16) sTimeTrialGhostData[frame].yaw * 0x100;
    ghost->oMoveAngleRoll       = (s16) sTimeTrialGhostData[frame].roll * 0x100;
    ghost->header.gfx.pos[0]    = (f32) sTimeTrialGhostData[frame].posX;
    ghost->header.gfx.pos[1]    = (f32) sTimeTrialGhostData[frame].posY;
    ghost->header.gfx.pos[2]    = (f32) sTimeTrialGhostData[frame].posZ;
    ghost->header.gfx.angle[0]  = (s16) sTimeTrialGhostData[frame].pitch * 0x100;
    ghost->header.gfx.angle[1]  = (s16) sTimeTrialGhostData[frame].yaw * 0x100;
    ghost->header.gfx.angle[2]  = (s16) sTimeTrialGhostData[frame].roll * 0x100;
    ghost->header.gfx.scale[0]  = (f32) sTimeTrialGhostData[frame].scaleX / 100.f;
    ghost->header.gfx.scale[1]  = (f32) sTimeTrialGhostData[frame].scaleY / 100.f;
    ghost->header.gfx.scale[2]  = (f32) sTimeTrialGhostData[frame].scaleZ / 100.f;
    ghost->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
    time_trials_ghost_update_animation(ghost, m, (u16) sTimeTrialGhostData[frame].animIndex, (u16) sTimeTrialGhostData[frame].animFrame);
}

static void time_trials_record_ghost(struct MarioState *m, s32 frame) {
    if (frame < 0 || frame >= TIME_TRIALS_MAX_ALLOWED_TIME) {
        return;
    }

    sTimeTrialGhostData[frame].posX      = (s16) m->marioObj->oPosX;
    sTimeTrialGhostData[frame].posY      = (s16) m->marioObj->oPosY;
    sTimeTrialGhostData[frame].posZ      = (s16) m->marioObj->oPosZ;
    sTimeTrialGhostData[frame].pitch     = (s8)(m->marioObj->oFaceAnglePitch / 0x100);
    sTimeTrialGhostData[frame].yaw       = (s8)(m->marioObj->oFaceAngleYaw / 0x100);
    sTimeTrialGhostData[frame].roll      = (s8)(m->marioObj->oFaceAngleRoll / 0x100);
    sTimeTrialGhostData[frame].scaleX    = (u8) MAX(0, MIN(255, m->marioObj->header.gfx.scale[0] * 100.f));
    sTimeTrialGhostData[frame].scaleY    = (u8) MAX(0, MIN(255, m->marioObj->header.gfx.scale[1] * 100.f));
    sTimeTrialGhostData[frame].scaleZ    = (u8) MAX(0, MIN(255, m->marioObj->header.gfx.scale[2] * 100.f));
    sTimeTrialGhostData[frame].animIndex = (u8) time_trials_ghost_retrieve_current_animation_index(m);
    sTimeTrialGhostData[frame].animFrame = (u8) MAX(0, MIN(255, MIN(m->marioObj->header.gfx.unk38.animFrame, m->marioObj->header.gfx.unk38.curAnim->unk08 - 1)));
    sTimeTrialGhostData[frame].level     = (u8) gCurrLevelNum;
    sTimeTrialGhostData[frame].area      = (u8) gCurrAreaIndex;
}

static void time_trials_ghost_unload() {
    struct Object *ghost = obj_get_time_trials_ghost();
    if (ghost != NULL) {
        obj_mark_for_deletion(ghost);
    }
}

//
// Timer
//

static s32 time_trials_get_ghost_data_star_to_load(s32 course, s32 star) {
    
    // Main courses
    if (course >= COURSE_BOB && course <= COURSE_RR) {
        return (TIME_TRIALS_MAIN_COURSE_STAR == 1 ? 6 : MAX(0, MIN(star, 5)));
    }

    // Bowser courses
    if (course >= COURSE_BITDW && course <= COURSE_BITS) {
        return (TIME_TRIALS_BOWSER_STAR == 1 ? 0 : 1);
    }

    // PSS
    if (course == COURSE_PSS) {
        return (TIME_TRIALS_PSS_STAR == 1 ? 1 : 0);
    }

    // Default
    return 0;
}

void time_trials_start_timer(struct MarioState *m, s32 fileIndex, s32 course, s32 star, s32 forceRestart) {
    static s32 sPrevCourse = -1;
    if (TIME_TRIALS == 1) {

        // Runs timer if course is TTable
        if (time_trials_get_slot_index(course, 0) != -1) {
            sTimeTrialTimerState = TT_TIMER_RUNNING;

            // Restart timer and init ghost data and model if different course
            if (forceRestart || (course != sPrevCourse)) {
                sTimeTrialTimer = 0;
                star = time_trials_get_ghost_data_star_to_load(course, star);
                if (time_trials_read_ghost_data(fileIndex, time_trials_get_slot_index(course, star), FALSE) > 0) {
                    time_trials_load_models();
                }
                if (forceRestart) {
                    m->health = sStartingMarioHealth;
                } else {
                    sStartingMarioHealth = m->health;
                }
            }
        } else {
            sTimeTrialTimerState = TT_TIMER_DISABLED;
        }
    }
    sTimeTrialHiScore = FALSE;
    sPrevCourse = course;
}

void time_trials_update_timer(struct MarioState *m) {

    // Disable timer and set timer to max time if Time Trials is disabled
    if (TIME_TRIALS == 0) {
        sTimeTrialTimerState = TT_TIMER_DISABLED;
        sTimeTrialTimer = TIME_TRIALS_MAX_ALLOWED_TIME;
        time_trials_ghost_unload();
    }

    // Update running ghost, record ghost data, and advance timer if it's running
    if (sTimeTrialTimerState == TT_TIMER_RUNNING) {
        sTimeTrialHiScore = FALSE;

        // Set to max time and stop Timer if Cheats are enabled
        if (Cheats.EnableCheats && TIME_TRIALS_CHEATS_ENABLED == 0) {
            time_trials_ghost_unload();
            sTimeTrialTimer = TIME_TRIALS_MAX_ALLOWED_TIME;
            sTimeTrialTimerState = TT_TIMER_STOPPED;
        } else {
            time_trials_update_ghost(m, sTimeTrialTimer, gCurrLevelNum, gCurrAreaIndex);
            time_trials_record_ghost(m, sTimeTrialTimer);
            sTimeTrialTimer = MIN(sTimeTrialTimer + 1, TIME_TRIALS_MAX_ALLOWED_TIME);
        }
    }
}

void time_trials_stop_timer() {
    if (sTimeTrialTimerState == TT_TIMER_RUNNING) {
        sTimeTrialTimerState = TT_TIMER_STOPPED;
    }
}

//
// Int to String
//

#define A 10,
#define B 11,
#define C 12,
#define D 13,
#define E 14,
#define F 15,
#define G 16,
#define H 17,
#define I 18,
#define J 19,
#define K 20,
#define L 21,
#define M 22,
#define N 23,
#define O 24,
#define P 25,
#define Q 26,
#define R 27,
#define S 28,
#define T 29,
#define U 30,
#define V 31,
#define W 32,
#define X 33,
#define Y 34,
#define Z 35,
#define a 36,
#define b 37,
#define c 38,
#define d 39,
#define e 40,
#define f 41,
#define g 42,
#define h 43,
#define i 44,
#define j 45,
#define k 46,
#define l 47,
#define m 48,
#define n 49,
#define o 50,
#define p 51,
#define q 52,
#define r 53,
#define s 54,
#define t 55,
#define u 56,
#define v 57,
#define w 58,
#define x 59,
#define y 60,
#define z 61,
#define APO 62,  // '
#define PER 63,  // .
#define COL 230, // :
#define COM 111, // ,
#define SLH 159, // -
#define DBQ 246, // "
#define COI 249, // coin
#define ST0 253, // empty star
#define ST1 250, // star filled
#define MUL 251, // x
#define _ DIALOG_CHAR_SPACE,

static const u8 *time_trials_time_to_string(s16 time) {
    static u8 buffer[16];

    if (time == TIME_TRIALS_UNDEFINED_TIME) {
        buffer[0] = SLH
        buffer[1] = APO
        buffer[2] = SLH
        buffer[3] = SLH
        buffer[4] = DBQ
        buffer[5] = SLH
        buffer[6] = SLH
        buffer[7] = DIALOG_CHAR_TERMINATOR;
        return buffer;
    }

    buffer[0] = (time / 1800) % 10;
    buffer[1] = APO
    buffer[2] = (time / 300) % 6;
    buffer[3] = (time / 30) % 10;
    buffer[4] = DBQ
    buffer[5] = (time / 3) % 10;
    buffer[6] = ((time * 10) / 3) % 10;
    buffer[7] = DIALOG_CHAR_TERMINATOR;
    return buffer;
}

static const u8 *time_trials_course_time_to_string(s32 coursetime) {
    static u8 buffer[16];

    if (coursetime == TIME_TRIALS_UNDEFINED_TIME) {
        buffer[0] = SLH
        buffer[1] = SLH
        buffer[2] = APO
        buffer[3] = SLH
        buffer[4] = SLH
        buffer[5] = DBQ
        buffer[6] = SLH
        buffer[7] = SLH
        buffer[8] = DIALOG_CHAR_TERMINATOR;
        return buffer;
    }

    buffer[0] = (coursetime / 18000) % 10;
    buffer[1] = (coursetime / 1800) % 10;
    buffer[2] = APO
    buffer[3] = (coursetime / 300) % 6;
    buffer[4] = (coursetime / 30) % 10;
    buffer[5] = DBQ
    buffer[6] = (coursetime / 3) % 10;
    buffer[7] = ((coursetime * 10) / 3) % 10;
    buffer[8] = DIALOG_CHAR_TERMINATOR;
    return buffer;
}

static const u8 *time_trials_total_time_to_string(s32 totalTime) {
    static u8 buffer[16];

    if (totalTime == TIME_TRIALS_UNDEFINED_TIME) {
        buffer[0] = SLH
        buffer[1] = SLH
        buffer[2] = COL
        buffer[3] = SLH
        buffer[4] = SLH
        buffer[5] = COL
        buffer[6] = SLH
        buffer[7] = SLH
        buffer[8] = PER
        buffer[9] = SLH
        buffer[10] = SLH
        buffer[11] = DIALOG_CHAR_TERMINATOR;
        return buffer;
    }

    buffer[0] = (totalTime / 1080000) % 10;
    buffer[1] = (totalTime / 108000) % 10;
    buffer[2] = COL
    buffer[3] = (totalTime / 18000) % 6;
    buffer[4] = (totalTime / 1800) % 10;
    buffer[5] = COL
    buffer[6] = (totalTime / 300) % 6;
    buffer[7] = (totalTime / 30) % 10;
    buffer[8] = PER
    buffer[9] = (totalTime / 3) % 10;
    buffer[10] = ((totalTime * 10) / 3) % 10;
    buffer[11] = DIALOG_CHAR_TERMINATOR;
    return buffer;
}

//
// Render
//

const u8 gTimeTrialsText100CoinsStar[]       = { 1,0,0,_ C O I N S _ S T A R             DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextStarFilled[]   = { ST1                                     DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextStarHollow[]   = { ST0                                     DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextDot[]          = { 55,                                     DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextApostrophe[]   = { 56,                                     DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextDoubleQuote[]  = { 57,                                     DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextPause[]        = { P A U S E                               DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextTime[]         = { T I M E                                 DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextCheater[]      = { D O N _ T _ B E _ A _ C H E A T E R     DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextHiScore[]      = { H I _ S C O R E                         DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextStar1[]        = { S T A R _ 1,                            DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextStar2[]        = { S T A R _ 2,                            DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextStar3[]        = { S T A R _ 3,                            DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextStar4[]        = { S T A R _ 4,                            DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextStar5[]        = { S T A R _ 5,                            DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextStar6[]        = { S T A R _ 6,                            DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextStar7[]        = { S T A R _ COI                           DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextKey1[]         = { K E Y _ 1,                              DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextKey2[]         = { K E Y _ 2,                              DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextFinal[]        = { F I N A L                               DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextCourse[]       = { C O U R S E                             DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextTotal[]        = { T O T A L                               DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextBowser[]       = { _ _ _ _ _ B O W S E R _ C O U R S E S   DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextPSS1[]         = { P S S _ 1,                              DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextPSS2[]         = { P S S _ 2,                              DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextSA[]           = { S A                                     DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextTOTWC[]        = { T O T W C                               DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextVCUTM[]        = { V C U T M                               DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextCOTMC[]        = { C O T M C                               DIALOG_CHAR_TERMINATOR };
static const u8 sTimeTrialTextWMOTR[]        = { W M O T R                               DIALOG_CHAR_TERMINATOR };
static const u8 *sTimeTrialTextStarGlyph[]   = { sTimeTrialTextStarHollow, sTimeTrialTextStarFilled };

struct RenderParams { const u8 *label; u8 starGlyph; s16 time; };
static const struct RenderParams sNoParam = { NULL, 0, 0 };

#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef H
#undef I
#undef J
#undef K
#undef L
#undef M
#undef N
#undef O
#undef P
#undef Q
#undef R
#undef S
#undef T
#undef U
#undef V
#undef W
#undef X
#undef Y
#undef Z
#undef a
#undef b
#undef c
#undef d
#undef e
#undef f
#undef g
#undef h
#undef i
#undef j
#undef k
#undef l
#undef m
#undef n
#undef o
#undef p
#undef q
#undef r
#undef s
#undef t
#undef u
#undef v
#undef w
#undef x
#undef y
#undef z
#undef APO
#undef PER
#undef COL
#undef COM
#undef SLH
#undef DBQ
#undef COI
#undef ST0
#undef ST1
#undef MUL
#undef _

/*
Position:
Title
1 5
2 6
3 7
4 8
C T
*/
static void time_trials_render_pause_times(struct RenderParams params[8], const u8 *title, s32 courseTime, s32 totalTime) {

    // Title
    print_generic_string(96, 168, title);

    // Stars
    for (s32 i = 0; i != 8; ++i) {
        if (params[i].label == NULL) {
            continue;
        }

        s16 x = 36 + 144 * (i / 4);
        s16 y = 144 - 24 * (i % 4);
        print_generic_string(x, y, params[i].label);
        print_generic_string(x + 46, y, sTimeTrialTextStarGlyph[params[i].starGlyph]);
        print_generic_string(x + 60, y, time_trials_time_to_string(params[i].time));
    }

    // Course time
    print_generic_string(36, 48, sTimeTrialTextCourse);
    print_generic_string(89, 48, time_trials_course_time_to_string(courseTime));

    // Total time
    print_generic_string(180, 48, sTimeTrialTextTotal);
    print_generic_string(217, 48, time_trials_total_time_to_string(totalTime));
}

static void time_trials_render_pause_course_times(s32 fileIndex, s32 course) {
    u8 starFlags = save_file_get_star_flags(fileIndex, course - COURSE_MIN);

    time_trials_render_pause_times((struct RenderParams[]) {
        (struct RenderParams) { sTimeTrialTextStar1, (starFlags >> 0) & 1, time_trials_get_time(fileIndex, course, 0) }, 
        (struct RenderParams) { sTimeTrialTextStar2, (starFlags >> 1) & 1, time_trials_get_time(fileIndex, course, 1) },
        (struct RenderParams) { sTimeTrialTextStar3, (starFlags >> 2) & 1, time_trials_get_time(fileIndex, course, 2) },
        sNoParam,
        (struct RenderParams) { sTimeTrialTextStar4, (starFlags >> 3) & 1, time_trials_get_time(fileIndex, course, 3) },
        (struct RenderParams) { sTimeTrialTextStar5, (starFlags >> 4) & 1, time_trials_get_time(fileIndex, course, 4) },
        (struct RenderParams) { sTimeTrialTextStar6, (starFlags >> 5) & 1, time_trials_get_time(fileIndex, course, 5) },
        (struct RenderParams) { sTimeTrialTextStar7, (starFlags >> 6) & 1, time_trials_get_time(fileIndex, course, 6) },
    },
        ((const u8 **) seg2_course_name_table)[course - COURSE_MIN],
        time_trials_get_course_time(fileIndex, course),
        time_trials_get_total_time(fileIndex)
    );
}

static void time_trials_render_pause_bowser_times(s32 fileIndex) {
    u8 bitdwStarFlags = save_file_get_star_flags(fileIndex, COURSE_BITDW - COURSE_MIN);
    u8 bitfsStarFlags = save_file_get_star_flags(fileIndex, COURSE_BITFS - COURSE_MIN);
    u8 bitsStarFlags  = save_file_get_star_flags(fileIndex, COURSE_BITS - COURSE_MIN);
    u8 bitdwKeyFlag   = save_file_get_flags() & (SAVE_FLAG_HAVE_KEY_1 | SAVE_FLAG_UNLOCKED_BASEMENT_DOOR);
    u8 bitfsKeyFlag   = save_file_get_flags() & (SAVE_FLAG_HAVE_KEY_2 | SAVE_FLAG_UNLOCKED_UPSTAIRS_DOOR);
    u8 bitsGrandStar  = time_trials_get_time(fileIndex, COURSE_BITS, 1) != TIME_TRIALS_UNDEFINED_TIME;

    time_trials_render_pause_times((struct RenderParams[]) {
        (struct RenderParams) { sTimeTrialTextStar1, (bitdwStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_BITDW, 0) }, 
        (struct RenderParams) { sTimeTrialTextStar2, (bitfsStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_BITFS, 0) },
        (struct RenderParams) { sTimeTrialTextStar3, (bitsStarFlags  >> 0) & 1, time_trials_get_time(fileIndex, COURSE_BITS,  0) },
        sNoParam,
        (struct RenderParams) { sTimeTrialTextKey1,  (bitdwKeyFlag)       != 0, time_trials_get_time(fileIndex, COURSE_BITDW, 1) },
        (struct RenderParams) { sTimeTrialTextKey2,  (bitfsKeyFlag)       != 0, time_trials_get_time(fileIndex, COURSE_BITFS, 1) },
        (struct RenderParams) { sTimeTrialTextFinal, (bitsGrandStar)      != 0, time_trials_get_time(fileIndex, COURSE_BITS,  1) },
        sNoParam,
    },
        sTimeTrialTextBowser,
        time_trials_get_bowser_time(fileIndex),
        time_trials_get_total_time(fileIndex)
    );
}

static void time_trials_render_pause_secret_times(s32 fileIndex) {
    u8 totwcStarFlags = save_file_get_star_flags(fileIndex, COURSE_TOTWC - COURSE_MIN);
    u8 vcutmStarFlags = save_file_get_star_flags(fileIndex, COURSE_VCUTM - COURSE_MIN);
    u8 cotmcStarFlags = save_file_get_star_flags(fileIndex, COURSE_COTMC - COURSE_MIN);
    u8 pssStarFlags   = save_file_get_star_flags(fileIndex, COURSE_PSS - COURSE_MIN);
    u8 saStarFlags    = save_file_get_star_flags(fileIndex, COURSE_SA - COURSE_MIN);
    u8 wmotrStarFlags = save_file_get_star_flags(fileIndex, COURSE_WMOTR - COURSE_MIN);

    time_trials_render_pause_times((struct RenderParams[]) {
        (struct RenderParams) { sTimeTrialTextPSS1,  (pssStarFlags   >> 0) & 1, time_trials_get_time(fileIndex, COURSE_PSS,   0) },
        (struct RenderParams) { sTimeTrialTextPSS2,  (pssStarFlags   >> 1) & 1, time_trials_get_time(fileIndex, COURSE_PSS,   1) },
        (struct RenderParams) { sTimeTrialTextSA,    (saStarFlags    >> 0) & 1, time_trials_get_time(fileIndex, COURSE_SA,    0) },
        (struct RenderParams) { sTimeTrialTextWMOTR, (wmotrStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_WMOTR, 0) },
        (struct RenderParams) { sTimeTrialTextTOTWC, (totwcStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_TOTWC, 0) }, 
        (struct RenderParams) { sTimeTrialTextVCUTM, (vcutmStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_VCUTM, 0) },
        (struct RenderParams) { sTimeTrialTextCOTMC, (cotmcStarFlags >> 0) & 1, time_trials_get_time(fileIndex, COURSE_COTMC, 0) },
        sNoParam,
    },
        ((const u8 **) seg2_course_name_table)[COURSE_MAX],
        time_trials_get_secret_time(fileIndex),
        time_trials_get_total_time(fileIndex)
    );
}

void time_trials_render_pause_castle_main_strings(s32 fileIndex, s8 *index) {
    s8 prevIndex = *index;
    handle_menu_scrolling(MENU_SCROLL_VERTICAL, index, -1, COURSE_STAGES_COUNT + 2);
    *index = (*index + COURSE_STAGES_COUNT + 2) % (COURSE_STAGES_COUNT + 2);
    
    // Hide the undiscovered courses
    if (*index < COURSE_STAGES_COUNT) {
        s8 inc = (*index >= prevIndex ? +1 : -1);
        while (save_file_get_course_star_count(fileIndex, *index) == 0) {
            *index += inc;
            if (*index == COURSE_STAGES_COUNT || *index == -1) {
                *index = COURSE_STAGES_COUNT;
                break;
            }
        }
    }

    // Render the colorful "PAUSE"
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    print_hud_lut_string(HUD_LUT_GLOBAL, 127, 14, sTimeTrialTextPause);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    // Render the black box behind the text
    create_dl_translation_matrix(MENU_MTX_PUSH, 16, 194, 0); // position from bottom-left
    create_dl_scale_matrix(MENU_MTX_NOPUSH, 2.215f, 1.925f, 1.0f); // 2.2*130=286px wide, 1.925*80=154px high # 2.21538
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 105);
    gSPDisplayList(gDisplayListHead++, dl_draw_text_bg_box);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    create_dl_translation_matrix(MENU_MTX_PUSH, 166, 198, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, 90.0f, 0, 0, 1.0f);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);
    create_dl_translation_matrix(MENU_MTX_PUSH, 151, 36, 0);
    create_dl_rotation_matrix(MENU_MTX_NOPUSH, 270.0f, 0, 0, 1.0f);
    gSPDisplayList(gDisplayListHead++, dl_draw_triangle);
    gSPPopMatrix(gDisplayListHead++, G_MTX_MODELVIEW);

    // Render the text
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, gDialogTextAlpha);
    switch (*index) {
        case COURSE_STAGES_COUNT:
            time_trials_render_pause_secret_times(fileIndex);
            break;
        case COURSE_STAGES_COUNT + 1:
            time_trials_render_pause_bowser_times(fileIndex);
            break;
        default:
            time_trials_render_pause_course_times(fileIndex, *index + COURSE_MIN);
            break;
    }
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

//
// HUD
//

static void time_trials_render_dont_be_a_cheater(s16 x, s16 y) {
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    print_hud_lut_string(HUD_LUT_GLOBAL, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(x), y, sTimeTrialTextCheater);
    print_hud_lut_string(HUD_LUT_GLOBAL, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(x - 33), y - 8, sTimeTrialTextApostrophe);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
}

static void time_trials_render_timer(s16 x, s16 y, s16 xOffset, const u8 *text, s16 time, u8 colorFade) {
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, colorFade, colorFade, colorFade, 255);
    print_hud_lut_string(HUD_LUT_GLOBAL, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(x + xOffset), y, text);
    print_hud_lut_string(HUD_LUT_GLOBAL, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(x - 60), y, time_trials_int_to_sm64_string((time / 1800) % 60, "%0d"));
    print_hud_lut_string(HUD_LUT_GLOBAL, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(x - 80), y, time_trials_int_to_sm64_string((time / 30) % 60, "%02d"));
    print_hud_lut_string(HUD_LUT_GLOBAL, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(x - 114), y, time_trials_int_to_sm64_string(((time * 10) / 3) % 100, "%02d"));
    print_hud_lut_string(HUD_LUT_GLOBAL, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(x - 70), y - 8, sTimeTrialTextApostrophe);
    print_hud_lut_string(HUD_LUT_GLOBAL, GFX_DIMENSIONS_RECT_FROM_RIGHT_EDGE(x - 105), y - 8, sTimeTrialTextDoubleQuote);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
}

void time_trials_render_hud_timer() {
    if (gHudDisplay.flags != HUD_DISPLAY_NONE && configHUD && sTimeTrialTimerState != TT_TIMER_DISABLED) {
        if (Cheats.EnableCheats && TIME_TRIALS_CHEATS_ENABLED == 0) {

            // Don't be a cheater
            time_trials_render_dont_be_a_cheater(324, 213);

        } else {
            const u8 *text;
            u8 colorFade;
            s16 xOffset;

            // Print a flashing HI SCORE if the recorded time is better than the previous one
            if (sTimeTrialHiScore && sTimeTrialTimerState == TT_TIMER_STOPPED) {
                text = sTimeTrialTextHiScore;
                colorFade = sins(gGlobalTimer * 0x1000) * 50.f + 205.f;
                xOffset = 44;
            } else {
                text = sTimeTrialTextTime;
                colorFade = 255;
                xOffset = 0;
            }
            time_trials_render_timer(294, 213, xOffset, text, sTimeTrialTimer, colorFade);
        }
    }
}

void time_trials_render_star_select_time(s32 fileIndex, s32 course, s32 star) {
    if (TIME_TRIALS == 1) {
        s16 time = time_trials_get_time(fileIndex, course, star);
        if (time != TIME_TRIALS_UNDEFINED_TIME) {
            time_trials_render_timer(294, 12, 0, sTimeTrialTextTime, time, 255);
        }
    }
}
