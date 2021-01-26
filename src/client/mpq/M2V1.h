/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2021 MaNGOS <https://getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#ifndef M2V1_H
#define M2V1_H

#include <string>
#include "Poco/Logger.h"
#include "MPQFile.h"
#include "G3D/AABox.h"
#include "G3D/Vector2.h"
#include "G3D/Vector3.h"
#include "G3D/Quat.h"

using Poco::Logger;
using G3D::AABox;
using G3D::Vector2;
using G3D::Vector3;
using G3D::Quat;

template<typename T>
struct M2Array
{
	unsigned int size;
	unsigned int offset;
};

struct M2TrackBase
{
	unsigned short interpolationType;
	unsigned short globalSequence;
	M2Array<std::pair<unsigned int, unsigned int>> interpolationRanges;
	M2Array<unsigned int> timestamps;
};

template<typename T>
struct M2Track : M2TrackBase
{
	M2Array<T> values;
};

template<typename T>
struct M2SplineKey
{
	T value;
	T inTan;
	T outTan;
};

struct M2Loop
{
	unsigned int timestamp;
};

struct M2Range
{
	unsigned int minimum;
	unsigned int maximum;
};

struct M2Bounds
{
	AABox boundingBox;
	float radius;
};

struct M2Sequence
{
	unsigned short id;
	unsigned short variationIdx;
	unsigned int startTimestamp;
	unsigned int endTimestamp;
	float moveSpeed;
	unsigned int flags;
	unsigned short frequency;
	unsigned short padding;
	M2Range replay;
	unsigned short blendTimeIn;
	unsigned short blendTimeOut;
	M2Bounds bounds;
	unsigned short variationNext;
	unsigned short aliasNext;
};

struct M2CompBone
{
	unsigned int keyBoneId;
	unsigned int enumBones;
	unsigned int flags;
	unsigned short parentBone;
	unsigned short submeshId;
	M2Track<Vector3> translation;
	M2Track<Quat> rotation;
	M2Track<Vector3> scale;
	Vector3 pivot;
};

struct M2Batch
{
	unsigned char flags;
	unsigned char priorityPlane;
	unsigned short shaderId;
	unsigned short skinSectionIndex;
	unsigned short geosetIndex;
	unsigned short colorIndex;
	unsigned short materialIndex;
	unsigned short materialLayer;
	unsigned short textureCount;
	unsigned short textureComboIndex;
	unsigned short textureCoordComboIndex;
	unsigned short textureWeightComboIndex;
	unsigned short textureTransformComboIndex;
};

struct M2Box
{
	Vector3 modelRotationSpeedMin;
	Vector3 modelRotationSpeedMax;
};

struct M2Vertex
{
	Vector3 position;
	unsigned char boneWeights[4];
	unsigned char boneIndices[4];
	Vector3 normal;
	Vector2 texCoords[2];
};

struct M2SkinSection
{
	unsigned short id;
	unsigned short level;
	unsigned short vertexStart;
	unsigned short vertexCount;
	unsigned int indexStart;
	unsigned short indexCount;
	unsigned short boneCount;
	unsigned short boneComboIndex;
	unsigned short boneInfluences;
	unsigned short centerBoneIndex;
	Vector3 centerPosition;
};

struct M2SkinProfile
{
	M2Array<unsigned short> vertices;
	M2Array<unsigned short> indices;
	M2Array<unsigned char> bones;
	M2Array<M2SkinSection> subMeshes;
	M2Array<M2Batch> listBatches;
};

struct M2Color
{
	M2Track<Vector3> color;
	M2Track<unsigned short> alpha;
};

struct M2Texture
{
	unsigned int type;
	unsigned int flags;
	M2Array<unsigned char> fileName;
};

struct M2TextureWeight
{
	M2Track<unsigned short> weight;
};

struct M2TextureTransform
{
	M2Track<Vector3> translation;
	M2Track<Quat> rotation;
	M2Track<Vector3> scaling;
};

struct M2Material
{
	unsigned short flags;
	unsigned short blendingMode;
};

struct M2Attachment
{
	unsigned int id;
	unsigned short bone;
	unsigned short unknown;
	Vector3 position;
};

struct M2Event
{
	unsigned int identifier;
	unsigned int data;
	unsigned int bone;
	Vector3 position;
	M2TrackBase enabled;
};

struct M2Light
{
	unsigned short type;
	unsigned short bone;
	Vector3 position;
	M2Track<Vector3> ambientColor;
	M2Track<float> ambientIntensity;
	M2Track<Vector3> diffuseColor;
	M2Track<float> diffuseIntensity;
	M2Track<float> attenuationStart;
	M2Track<float> attenuationEnd;
	M2Track<unsigned char> visiblity;
};

struct M2Camera
{
	unsigned int type;
	float fov;
	float farClip;
	float nearClip;
	M2Track<M2SplineKey<Vector3>> positions;
	Vector3 positionBase;
	M2Track<M2SplineKey<Vector3>> targetPosition;
	Vector3 targetPositionBase;
	M2Track<M2SplineKey<float>> roll;
};

struct M2Ribbon
{
	unsigned int ribbonId;
	unsigned int boneIndex;
	Vector3 position;
	M2Array<unsigned short> textureIndices;
	M2Array<unsigned short> materialIndices;
	M2Track<Vector3> colorTrack;
	M2Track<unsigned short> alphaTrack;
	M2Track<float> heightAboveTrack;
	M2Track<float> heightBelowTrack;
	float edgesPerSecond;
	float edgeLifeTime;
	float gravity;
	short textureRows;
	short textureCols;
	M2Track<unsigned short> texSlotTrack;
	M2Track<bool> visibilityTrack;
};

struct M2Particle
{
	unsigned int particleId;
	unsigned int flags;
	Vector3 position;
	unsigned short bone;
	unsigned short texture;
	M2Array<unsigned char> geometryModelFilename;
	M2Array<unsigned char> recursionModelFilename;
	unsigned short blendingType;
	unsigned short emitterType;
	unsigned char particleType;
	unsigned char headorTail;
	unsigned short textureTileRotation;
	unsigned short textureDimensionsRows;
	unsigned short textureDimensionsColumns;
	M2Track<float> emissionSpeed;
	M2Track<float> speedVariation;
	M2Track<float> verticalRange;
	M2Track<float> horizontalRange;
	M2Track<float> gravity;
	M2Track<float> lifespan;
	M2Track<float> emissionRate;
	M2Track<float> emissionAreaLength;
	M2Track<float> emissionAreaWidth;
	M2Track<float> zSource;
	float midPoint;
	unsigned char colorValues[4][3];
	float scaleValues[3];
	unsigned short headCellBegin[2];
	unsigned short between1;
	unsigned short headCellEnd[2];
	unsigned short between2;
	unsigned short tiles[4];
	float tailLength;
	float twinkleSpeed;
	float twinklePercent;
	Vector2 twinkleScale;
	float burstMultiplier;
	float drag;
	float spin;
	M2Box tumble;
	Vector3 windVector;
	float windTime;
	float followSpeed1;
	float followScale1;
	float followSpeed2;
	float followScale2;
	M2Array<Vector3> splinePoints;
	M2Track<bool> enableInd;
};

class M2V1 : public MPQFile
{
public:
	M2V1(std::string name, unsigned char* data, long size);
	~M2V1();
	bool parse();

	// Full model system.
	unsigned int getNVertices();
	M2Vertex* getVertices();
	M2SkinProfile* getSkins();
	M2SkinSection* getSubmeshes(unsigned int view);
	unsigned short* getIndices(unsigned int view);

	// Collision system.
	unsigned int const getNCollisionVertices();
	const Vector3* getCollisionVertices();
	const unsigned int getNCollisionTriangles();
	const unsigned short* getCollisionTriangles();

private:
	/** File version - MD20 chunk */
	const std::string HEADER_MD20 = "MD20";
	const unsigned int SUPPORTED_VERSIONS[2] = { 256, 257 };

	struct Header {
		char magic[4];
		unsigned int version;
		M2Array<unsigned char> name;
		unsigned int flags;
		M2Array<M2Loop> globalLoops;
		M2Array<M2Sequence> sequences;
		M2Array<unsigned short> sequenceLookups;
		M2Array<M2Sequence> playableAnimationLookup;
		M2Array<M2CompBone> bones;
		M2Array<unsigned short> keyBoneLookup;
		M2Array<M2Vertex> vertices;
		M2Array<M2SkinProfile> skinProfiles;
		M2Array<M2Color> colors;
		M2Array<M2Texture> textures;
		M2Array<M2TextureWeight> textureWeights;
		M2Array<unsigned short> textureFlipbooks;
		M2Array<M2TextureTransform> textureTransforms;
		M2Array<unsigned short> replacableTextureLookup;
		M2Array<M2Material> materials;
		M2Array<unsigned short> boneLookupTable;
		M2Array<unsigned short> textureLookupTable;
		M2Array<unsigned short> texUnitLookupTable;
		M2Array<unsigned short> transparencyLookupTable;
		M2Array<unsigned short> textureTransformsLookupTable;
		AABox boundingBox;
		float boundingSphereRadius;
		AABox collisionBox;
		float collisionSphereRadius;
		M2Array<unsigned short> collisionTriangles;
		M2Array<Vector3> collisionVertices;
		M2Array<Vector3> collisionNormals;
		M2Array<M2Attachment> attachments;
		M2Array<unsigned short> attachmentLookupTable;
		M2Array<M2Event> events;
		M2Array<M2Light> lights;
		M2Array<M2Camera> cameras;
		M2Array<unsigned short> cameraLookupTable;
		M2Array<M2Ribbon> ribbonEmitters;
		M2Array<M2Particle> particleEmitters;
		M2Array<unsigned short> textureCombinerCombos;

	} * _header;
	
};

#endif