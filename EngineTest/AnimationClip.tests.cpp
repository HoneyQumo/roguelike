#include "pch.h"
#include "AnimationClip.h"

using XYZEngine::AnimationClip;

TEST(AnimationClipTests, EmptyClipHasNoFrames)
{
	AnimationClip clip;

	EXPECT_TRUE(clip.IsEmpty());
	EXPECT_EQ(clip.GetFramesCount(), 0);
	EXPECT_EQ(clip.GetFrame(0), nullptr);
}

TEST(AnimationClipTests, UnknownTextureMapIsNotLoaded)
{
	AnimationClip clip;

	EXPECT_FALSE(clip.Load("no_such_texture_map", 0, 4, 10.f));
	EXPECT_TRUE(clip.IsEmpty());
}

TEST(AnimationClipTests, FramerateMustBePositive)
{
	AnimationClip clip;

	EXPECT_FALSE(clip.Load("no_such_texture_map", 0, 4, 0.f));
	EXPECT_FALSE(clip.Load("no_such_texture_map", 0, 4, -1.f));
	EXPECT_TRUE(clip.IsEmpty());
}

TEST(AnimationClipTests, EmptyClipKeepsItsDefaultFrameLength)
{
	AnimationClip clip;
	float defaultSeconds = clip.GetFrameSeconds(0);

	clip.Load("no_such_texture_map", 0, 4, 20.f);

	EXPECT_FLOAT_EQ(clip.GetFrameSeconds(0), defaultSeconds);
	EXPECT_FLOAT_EQ(clip.GetFrameSeconds(100), defaultSeconds);
	EXPECT_FLOAT_EQ(clip.GetFrameSeconds(-5), defaultSeconds);
}
