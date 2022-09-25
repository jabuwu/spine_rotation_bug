/******************************************************************************
 * Spine Runtimes License Agreement
 * Last updated September 24, 2021. Replaces all prior versions.
 *
 * Copyright (c) 2013-2021, Esoteric Software LLC
 *
 * Integration of the Spine Runtimes into software or otherwise creating
 * derivative works of the Spine Runtimes is permitted under the terms and
 * conditions of Section 2 of the Spine Editor License Agreement:
 * http://esotericsoftware.com/spine-editor-license
 *
 * Otherwise, it is permitted to integrate the Spine Runtimes into software
 * or otherwise create derivative works of the Spine Runtimes (collectively,
 * "Products"), provided that each user of the Products must obtain their own
 * Spine Editor license and redistribution of the Products in any form must
 * include this license and copyright notice.
 *
 * THE SPINE RUNTIMES ARE PROVIDED BY ESOTERIC SOFTWARE LLC "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ESOTERIC SOFTWARE LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES,
 * BUSINESS INTERRUPTION, OR LOSS OF USE, DATA, OR PROFITS) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THE SPINE RUNTIMES, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <iostream>
#include <spine/Debug.h>
#include <spine/spine-sfml.h>

using namespace std;
using namespace spine;
#include <stdio.h>
#include <stdlib.h>

void callback(spAnimationState *state, spEventType type, spTrackEntry *entry, spEvent *event) {
	UNUSED(state);
	const char *animationName = (entry && entry->animation) ? entry->animation->name : 0;

	switch (type) {
		case SP_ANIMATION_START:
			printf("%d start: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_INTERRUPT:
			printf("%d interrupt: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_END:
			printf("%d end: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_COMPLETE:
			printf("%d complete: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_DISPOSE:
			printf("%d dispose: %s\n", entry->trackIndex, animationName);
			break;
		case SP_ANIMATION_EVENT:
			printf("%d event: %s, %s: %d, %f, %s %f %f\n", entry->trackIndex, animationName, event->data->name, event->intValue, event->floatValue,
				   event->stringValue, event->volume, event->balance);
			break;
	}
	fflush(stdout);
}

spSkeletonData *readSkeletonJsonData(const char *filename, spAtlas *atlas, float scale) {
	spSkeletonJson *json = spSkeletonJson_create(atlas);
	json->scale = scale;
	spSkeletonData *skeletonData = spSkeletonJson_readSkeletonDataFile(json, filename);
	if (!skeletonData) {
		printf("%s\n", json->error);
		exit(0);
	}
	spSkeletonJson_dispose(json);
	return skeletonData;
}

spSkeletonData *readSkeletonBinaryData(const char *filename, spAtlas *atlas, float scale) {
	spSkeletonBinary *binary = spSkeletonBinary_create(atlas);
	binary->scale = scale;
	spSkeletonData *skeletonData = spSkeletonBinary_readSkeletonDataFile(binary, filename);
	if (!skeletonData) {
		printf("%s\n", binary->error);
		exit(0);
	}
	spSkeletonBinary_dispose(binary);
	return skeletonData;
}

void testcase(void func(spSkeletonData *skeletonData, spAtlas *atlas),
			  const char *jsonName, const char *binaryName, const char *atlasName,
			  float scale) {
	spAtlas *atlas = spAtlas_createFromFile(atlasName, 0);

	spSkeletonData *skeletonData = readSkeletonBinaryData(binaryName, atlas, scale);
	func(skeletonData, atlas);
	spSkeletonData_dispose(skeletonData);

	skeletonData = readSkeletonJsonData(jsonName, atlas, scale);
	func(skeletonData, atlas);
	spSkeletonData_dispose(skeletonData);

	spAtlas_dispose(atlas);

	UNUSED(jsonName);
}

void spineboy(spSkeletonData *skeletonData, spAtlas *atlas) {
	UNUSED(atlas);
	spSkeletonBounds *bounds = spSkeletonBounds_create();

	// Configure mixing.
	spAnimationStateData *stateData = spAnimationStateData_create(skeletonData);
	spAnimationStateData_setMixByName(stateData, "walk", "jump", 0.2f);
	spAnimationStateData_setMixByName(stateData, "jump", "run", 0.2f);

	SkeletonDrawable *drawable = new SkeletonDrawable(skeletonData, stateData);
	drawable->timeScale = 1;
	drawable->setUsePremultipliedAlpha(true);

	spSkeleton *skeleton = drawable->skeleton;
	spSkeleton_setToSetupPose(skeleton);

	skeleton->x = 320;
	skeleton->y = 590;
	spSkeleton_updateWorldTransform(skeleton);

	spSlot *headSlot = spSkeleton_findSlot(skeleton, "head");

	drawable->state->listener = callback;
	spAnimationState_setAnimationByName(drawable->state, 0, "idle", true);
	spAnimationState_setAnimationByName(drawable->state, 1, "run", true);

	for (int i = 0; i < 700; ++i) {
		drawable->state->tracks[1]->alpha = (i % 200 > 100) ? 0.5 : 1.0;
		spAnimationState_update(drawable->state, 0.016);
		spAnimationState_apply(drawable->state, drawable->skeleton);
		spSkeleton_updateWorldTransform(drawable->skeleton);
	}
	drawable->state->tracks[1]->alpha = 0.5;
	spAnimationState_update(drawable->state, 0.016);
	spAnimationState_apply(drawable->state, drawable->skeleton);
	spSkeleton_updateWorldTransform(drawable->skeleton);
	for (int i = 0; i < 2000; ++i) {
		spAnimationState_update(drawable->state, 0.016);
		spAnimationState_apply(drawable->state, drawable->skeleton);
		spSkeleton_updateWorldTransform(drawable->skeleton);
	}

	sf::RenderWindow window(sf::VideoMode(640, 640), "Spine SFML - spineboy");
	window.setFramerateLimit(60);
	sf::Event event;
	sf::Clock deltaClock;
	while (window.isOpen()) {
		while (window.pollEvent(event))
			if (event.type == sf::Event::Closed) window.close();

		float delta = deltaClock.getElapsedTime().asSeconds();
		deltaClock.restart();

		spSkeletonBounds_update(bounds, skeleton, true);
		sf::Vector2i position = sf::Mouse::getPosition(window);
		if (spSkeletonBounds_containsPoint(bounds, (float) position.x, (float) position.y)) {
			headSlot->color.g = 0;
			headSlot->color.b = 0;
		} else {
			headSlot->color.g = 1;
			headSlot->color.b = 1;
		}

		drawable->update(delta);

		window.clear();
		window.draw(*drawable);
		window.display();
	}

	spSkeletonBounds_dispose(bounds);
}

int main() {
	testcase(spineboy, "../spine-runtimes/examples/spineboy/export/spineboy-pro.json", "../spine-runtimes/examples/spineboy/export/spineboy-pro.skel", "../spine-runtimes/examples/spineboy/export/spineboy-pma.atlas", 0.6f);
	return 0;
}
