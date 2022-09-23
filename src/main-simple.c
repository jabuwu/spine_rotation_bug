#include <spine/spine.h>
#include <spine/extension.h>
#include <stdio.h>

int main() {
    spAtlas* atlas = spAtlas_createFromFile("../spineboy/export/spineboy.atlas", 0);
    spSkeletonJson* skeleton_json = spSkeletonJson_create(atlas);
    spSkeletonData* skeleton_data = spSkeletonJson_readSkeletonDataFile(skeleton_json, "../spineboy/export/spineboy-pro.json");
    spAnimationStateData* animation_state_data = spAnimationStateData_create(skeleton_data);
    spSkeleton* skeleton = spSkeleton_create(skeleton_data);
    spAnimationState* animation_state = spAnimationState_create(animation_state_data);
    spAnimationState_setAnimationByName(animation_state, 0, "run", 1);

    for (int i = 0; i < 700; ++i) {
        animation_state->tracks[0]->alpha = (i % 200 > 100) ? 0.5 : 1.0;
        spAnimationState_update(animation_state, 0.016);
        spAnimationState_apply(animation_state, skeleton);
        spSkeleton_updateWorldTransform(skeleton);
    }
    animation_state->tracks[0]->alpha = 0.5;
    spAnimationState_update(animation_state, 0.016);
    spAnimationState_apply(animation_state, skeleton);
    spSkeleton_updateWorldTransform(skeleton);
    for (int i = 0; i < 2000; ++i) {
        spAnimationState_update(animation_state, 0.016);
        spAnimationState_apply(animation_state, skeleton);
        spSkeleton_updateWorldTransform(skeleton);
    }

    spBone* front_foot = spSkeleton_findBone(skeleton, "front-foot-tip");

    printf("%f\n", front_foot->arotation);
}
