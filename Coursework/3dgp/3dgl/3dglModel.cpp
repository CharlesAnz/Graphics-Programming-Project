#include <iostream>
#include <fstream>
#include "../GL/glew.h"
#include "../GL/3dglModel.h"
#include "../GL/3dglShader.h"
#include "../GL/3dglBitmap.h"

// assimp include file
#include "../GL/assimp/cimport.h"

// GLM include files
#include "../glm/vec3.hpp"
#include "../glm/vec4.hpp"
#include "../glm/mat4x4.hpp"
#include "../glm/trigonometric.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include <assert.h>

using namespace std;
using namespace _3dgl;

unsigned C3dglModel::MATERIAL::c_idTexBlank = 0xFFFFFFFF;

bool C3dglModel::load(const char* pFile, unsigned int flags)
{
	logInfo(string("Importing file: ") + pFile);
	const aiScene *pScene = aiImportFile(pFile, flags);
	if (pScene == NULL) return false;
	m_name = pFile;
	size_t i = m_name.find_last_of("/\\");
	if (i != string::npos) m_name = m_name.substr(i + 1);
	i = m_name.find_last_of(".");
	if (i != string::npos) m_name = m_name.substr(0, i);
	create(pScene);
	return true;
}

void C3dglModel::MESH::create(const aiMesh *pMesh, unsigned maskEnabledBufData)
{
	if (pMesh->mFaces[0].mNumIndices != 3 && pMesh->mNumFaces && pMesh->mNumVertices && pMesh->mVertices && pMesh->mNormals)
		return;

	// find the BB (bounding box)
	bb[0] = bb[1] = pMesh->mVertices[0];
	for (aiVector3D vec : vector<aiVector3D>(pMesh->mVertices, pMesh->mVertices + pMesh->mNumVertices))
	{
		if (vec.x < bb[0].x) bb[0].x = vec.x;
		if (vec.y < bb[0].y) bb[0].y = vec.y;
		if (vec.z < bb[0].z) bb[0].z = vec.z;
		if (vec.x > bb[0].x) bb[1].x = vec.x;
		if (vec.y > bb[0].y) bb[1].y = vec.y;
		if (vec.z > bb[0].z) bb[1].z = vec.z;
	}
	centre.x = 0.5f * (bb[0].x + bb[1].x);
	centre.y = 0.5f * (bb[0].y + bb[1].y);
	centre.z = 0.5f * (bb[0].z + bb[1].z);

	// check shader parameters
	GLuint attribVertex = (GLuint)-1, attribNormal = (GLuint)-1, attribTexCoord = (GLuint)-1, 
		   attribTangent = (GLuint)-1, attribBitangent = (GLuint)-1, attribColor = (GLuint)-1,
		   attribBoneId = (GLuint)-1, attribBoneWeight = (GLuint)-1;
	
	C3dglProgram *pProgram = C3dglProgram::GetCurrentProgram();
	if (pProgram)
	{
		attribVertex    = pProgram->GetAttribLocation(C3dglProgram::ATTR_VERTEX);
		attribNormal    = pProgram->GetAttribLocation(C3dglProgram::ATTR_NORMAL);
		attribTexCoord  = pProgram->GetAttribLocation(C3dglProgram::ATTR_TEXCOORD);
		attribTangent   = pProgram->GetAttribLocation(C3dglProgram::ATTR_TANGENT);
		attribBitangent = pProgram->GetAttribLocation(C3dglProgram::ATTR_BITANGENT);
		attribColor     = pProgram->GetAttribLocation(C3dglProgram::ATTR_COLOR);
		attribBoneId	= pProgram->GetAttribLocation(C3dglProgram::ATTR_BONE_ID);
		attribBoneWeight= pProgram->GetAttribLocation(C3dglProgram::ATTR_BONE_WEIGHT);
	}
	else
		attribVertex = attribNormal = attribTexCoord = 0;	// enabled by default if no shader used

	// create VAO
	glGenVertexArrays(1, &m_idVAO);
	glBindVertexArray(m_idVAO);

	// generate a vertex buffer, than bind it and send data to OpenGL
	if (attribVertex != (GLuint)-1)
		if (pMesh->mVertices)
		{
			m_buf[BUF_VERTEX].populate(sizeof(pMesh->mVertices[0]), pMesh->mNumVertices, &pMesh->mVertices[0]);
			if (maskEnabledBufData & (1 << BUF_VERTEX)) 
				m_buf[BUF_VERTEX].storeData(sizeof(pMesh->mVertices[0]), pMesh->mNumVertices, &pMesh->mVertices[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribVertex);
				glVertexAttribPointer(attribVertex, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
			else
			{
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, 0, 0);
			}
		}
		else
			m_pOwner->logWarning("is missing vertex buffer information");

	// generate a normal buffer, than bind it and send data to OpenGL
	if (attribNormal != (GLuint)-1)
		if (pMesh->mNormals)
		{
			m_buf[BUF_NORMAL].populate(sizeof(pMesh->mNormals[0]), pMesh->mNumVertices, &pMesh->mNormals[0]);
			if (maskEnabledBufData & (1 << BUF_NORMAL))
				m_buf[BUF_NORMAL].storeData(sizeof(pMesh->mNormals[0]), pMesh->mNumVertices, &pMesh->mNormals[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribNormal);
				glVertexAttribPointer(attribNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
			else
			{
				glEnableClientState(GL_NORMAL_ARRAY);
				glNormalPointer(GL_FLOAT, 0, 0);
			}
		}
		else
			m_pOwner->logWarning("is missing normal buffer information");

	//Texture Coordinates
	if (attribTexCoord != (GLuint)-1)
	{
		m_nUVComponents = pMesh->mNumUVComponents[0];	// should be 2
		if (pMesh->mTextureCoords[0] == NULL)
			m_pOwner->logWarning("is missing texture coordinate buffer information");
		else if (m_nUVComponents != 2 && m_nUVComponents != 3)
			m_pOwner->logWarning("is missing compatible texture coordinates");
		else
		{
			// first, convert indices to occupy contageous memory space
			vector<GLfloat> texCoords;
			for (aiVector3D vec : vector<aiVector3D>(pMesh->mTextureCoords[0], pMesh->mTextureCoords[0] + pMesh->mNumVertices))
			{
				texCoords.push_back(vec.x);
				texCoords.push_back(vec.y);
				if (m_nUVComponents == 3)
					texCoords.push_back(vec.z);
			}

			m_buf[BUF_TEXCOORD].populate(sizeof(texCoords[0]), texCoords.size(), &texCoords[0]);
			if (maskEnabledBufData & (1 << BUF_TEXCOORD))
				m_buf[BUF_TEXCOORD].storeData(sizeof(texCoords[0]), texCoords.size(), &texCoords[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribTexCoord);
				glVertexAttribPointer(attribTexCoord, m_nUVComponents, GL_FLOAT, GL_FALSE, 0, 0);
			}
			else
			{
				glEnableClientState( GL_TEXTURE_COORD_ARRAY);
				glTexCoordPointer(m_nUVComponents, GL_FLOAT, 0, 0);
			}
		}
	}

	// generate a tangent buffer, than bind it and send data to OpenGL
	if (attribTangent != (GLuint)-1)
		if (pMesh->mTangents)
		{
			m_buf[BUF_TANGENT].populate(sizeof(pMesh->mTangents[0]), pMesh->mNumVertices, &pMesh->mTangents[0]);
			if (maskEnabledBufData & (1 << BUF_TANGENT))
				m_buf[BUF_TANGENT].storeData(sizeof(pMesh->mTangents[0]), pMesh->mNumVertices, &pMesh->mTangents[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribTangent);
				glVertexAttribPointer(attribTangent, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}
		else
			m_pOwner->logWarning("is missing tangent buffer information");

	// generate a biTangent buffer, than bind it and send data to OpenGL
	if (attribBitangent != (GLuint)-1)
		if (pMesh->mBitangents)
		{
			m_buf[BUF_BITANGENT].populate(sizeof(pMesh->mBitangents[0]), pMesh->mNumVertices, &pMesh->mBitangents[0]);
			if (maskEnabledBufData & (1 << BUF_BITANGENT))
				m_buf[BUF_BITANGENT].storeData(sizeof(pMesh->mBitangents[0]), pMesh->mNumVertices, &pMesh->mBitangents[0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribBitangent);
				glVertexAttribPointer(attribBitangent, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}
		else
			m_pOwner->logWarning("is missing bitangent buffer information");

	// generate a color buffer, than bind it and send data to OpenGL
	if (attribColor != (GLuint)-1)
		if (pMesh->mColors[0])
		{
			m_buf[BUF_COLOR].populate(sizeof(pMesh->mColors[0][0]), pMesh->mNumVertices, &pMesh->mColors[0][0]);
			if (maskEnabledBufData & (1 << BUF_COLOR))
				m_buf[BUF_COLOR].storeData(sizeof(pMesh->mColors[0][0]), pMesh->mNumVertices, &pMesh->mColors[0][0]);

			if (pProgram)
			{
				glEnableVertexAttribArray(attribColor);
				glVertexAttribPointer(attribColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
			}
		}
		else
			m_pOwner->logWarning("is missing color buffer information");

	// generate and convert a bone buffer, than bind it and send data to OpenGL
	if (attribBoneId != (GLuint)-1 && attribBoneWeight != (GLuint)-1)
	{
		// initislise the buffer
		struct VertexBoneData
		{        
			unsigned ids[MAX_BONES_PER_VEREX];
			float weights[MAX_BONES_PER_VEREX];
		};
		vector<VertexBoneData> bones;
		bones.resize(pMesh->mNumVertices);
		memset(&bones[0], 0, sizeof(bones[0]) * bones.size());

		if (pMesh->mNumBones)
		{
			// load bone info - based on http://ogldev.atspace.co.uk/
			m_pOwner->logInfo("bones found: " + to_string(pMesh->mNumBones));

			// for each bone:
			for (aiBone *pBone : vector<aiBone*>(pMesh->mBones, pMesh->mBones + pMesh->mNumBones))
			{
				// determine bone index from its name
				unsigned iBone = m_pOwner->getBoneId(pBone->mName.data);

				if (iBone >= m_pOwner->m_offsetBones.size())
					m_pOwner->m_offsetBones.push_back(pBone->mOffsetMatrix);
				
				// collect bone weights
				for (aiVertexWeight &weight : vector<aiVertexWeight>(pBone->mWeights, pBone->mWeights + pBone->mNumWeights))
				{
					// find a free location for the id and weight within bones[iVertex]
					unsigned i = 0;
					while (i < MAX_BONES_PER_VEREX && bones[weight.mVertexId].weights[i] != 0.0)
						i++;
					if (i < MAX_BONES_PER_VEREX)
					{
						bones[weight.mVertexId].ids[i] = iBone;
						bones[weight.mVertexId].weights[i] = weight.mWeight;
					}
					else
						m_pOwner->logWarning("Maximum number of bones per vertex exceeded");
				}
			}

			// verify (and maybe, in future, normalize)
			bool bProblem = false;
			for (VertexBoneData &bone : bones)
			{
				float total = 0.0f;
				for (float weight : bone.weights)
					total += weight;
				bProblem = bProblem || total < 0.999f || total > 1.001f;
				//cout << total << endl;
			}
			if (bProblem)
				m_pOwner->logWarning("Some bone weights do not sum up to 1.0");
		}
		else
			m_pOwner->logWarning("is missing bone information");

		m_buf[BUF_BONE].populate(sizeof(bones[0]), pMesh->mNumVertices, &bones[0]);
		if (maskEnabledBufData & (1 << BUF_BONE))
			m_buf[BUF_BONE].storeData(sizeof(bones[0]), pMesh->mNumVertices, &bones[0]);

		if (pProgram)
		{
			glEnableVertexAttribArray(attribBoneId);
			glVertexAttribIPointer(attribBoneId, 4, GL_INT, sizeof(VertexBoneData), (const GLvoid*)0);
			glEnableVertexAttribArray(attribBoneWeight); 
			glVertexAttribPointer(attribBoneWeight, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (const GLvoid*)sizeof(bones[0].ids));
		}
	}

	// first, convert indices to occupy contageous memory space
	vector<unsigned> indices;
	for (aiFace f : vector<aiFace>(pMesh->mFaces, pMesh->mFaces + pMesh->mNumFaces))
		for (unsigned n : vector<unsigned>(f.mIndices, f.mIndices + f.mNumIndices))
			indices.push_back(n);

	// generate indices buffer, than bind it and send data to OpenGL
	m_buf[BUF_INDEX].populate(sizeof(indices[0]), indices.size(), &indices[0], GL_ELEMENT_ARRAY_BUFFER);
	if (maskEnabledBufData & (1 << BUF_INDEX))
		m_buf[BUF_INDEX].storeData(sizeof(indices[0]), indices.size(), &indices[0]);
	m_indexSize = indices.size();

	m_nMaterialIndex = pMesh->mMaterialIndex;

	// Reset VAO & buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void C3dglModel::MESH::destroy()
{
	m_buf[BUF_VERTEX].release();
	m_buf[BUF_NORMAL].release();
	m_buf[BUF_TEXCOORD].release();
	m_buf[BUF_TANGENT].release();
	m_buf[BUF_BITANGENT].release();
	m_buf[BUF_COLOR].release();
	m_buf[BUF_BONE].release();
	m_buf[BUF_INDEX].release();
}

void C3dglModel::MESH::render() 
{
	glBindVertexArray(m_idVAO);
	glDrawElements(GL_TRIANGLES, m_indexSize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

C3dglModel::MATERIAL *C3dglModel::MESH::createNewMaterial()
{
	C3dglModel::MATERIAL mat(m_pOwner);
	m_nMaterialIndex = m_pOwner->m_materials.size();
	m_pOwner->m_materials.push_back(mat);
	return getMaterial();
}

C3dglModel::MATERIAL::MATERIAL(C3dglModel *pOwner) : m_pOwner(pOwner)
{
	m_idTexture = 0xFFFFFFFF;
	memset(m_amb, 0, sizeof(m_amb));;
	memset(m_diff, 0, sizeof(m_diff));;
	memset(m_spec, 0, sizeof(m_spec));;
	memset(m_emiss, 0, sizeof(m_emiss));;
	m_shininess = 0.0f;
}

void C3dglModel::MATERIAL::create(const aiMaterial *pMat, const char* pDefTexPath)
{
	// texture
	aiString texPath;	// contains filename of texture
	if (pMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
		loadTexture(pDefTexPath ? pDefTexPath : "", texPath.C_Str());
	if (m_idTexture == 0xFFFFFFFF)
		loadBlankTexture();

	// solid colours
	static float def_amb[3] = { 1.0, 1.0, 1.0 };
	static float def_diff[3] = { 1.0, 1.0, 1.0 };
	static float def_spec[3] = { 0.0, 0.0, 0.0 };
	static float def_emiss[3] = { 0.0, 0.0, 0.0 };
	static float def_shininess = 0.1f;
	aiColor4D color;
	if(AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_AMBIENT, &color))
		memcpy(this->m_amb, &color, sizeof(m_amb));
	else
		memcpy(this->m_amb, &def_amb, sizeof(m_amb));
	if(AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_DIFFUSE, &color))
		memcpy(this->m_diff, &color, sizeof(m_diff));
	else
		memcpy(this->m_diff, &def_diff, sizeof(m_diff));
	if(AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_SPECULAR, &color))
		memcpy(this->m_spec, &color, sizeof(m_spec));
	else
		memcpy(this->m_spec, &def_spec, sizeof(m_spec));
	if(AI_SUCCESS == aiGetMaterialColor(pMat, AI_MATKEY_COLOR_EMISSIVE, &color))
		memcpy(this->m_emiss, &color, sizeof(m_emiss));
	else
		memcpy(this->m_emiss, &def_emiss, sizeof(m_emiss));
	float shininess;
	unsigned int max;
	if(AI_SUCCESS == aiGetMaterialFloatArray(pMat, AI_MATKEY_SHININESS, &shininess, &max))
		this->m_shininess = shininess;
	else
		this->m_shininess = def_shininess;
}

void C3dglModel::MATERIAL::destroy()
{
	if (m_idTexture != 0xffffffff)
		glDeleteTextures(1, &m_idTexture);
}

void C3dglModel::MATERIAL::bind()
{
	if (m_idTexture != 0xffffffff)
		glBindTexture(GL_TEXTURE_2D, m_idTexture);

	// check if a shading program is active
	C3dglProgram *pProgram = C3dglProgram::GetCurrentProgram();
	if (pProgram)
	{
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_AMBIENT, m_amb[0], m_amb[1], m_amb[2]);	
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_DIFFUSE, m_diff[0], m_diff[1], m_diff[2]);
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_SPECULAR, m_spec[0], m_spec[1], m_spec[2]);
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_EMISSIVE, m_emiss[0], m_emiss[1], m_emiss[2]);
		pProgram->SendStandardUniform(C3dglProgram::UNI_MAT_SHININESS, m_shininess);
	}
}

void C3dglModel::MATERIAL::loadTexture(string strDefTexPath, string strPath)
{
	// prepare path
	std::ifstream f(strPath);
	if (f.is_open())
	{
		f.close();
	}
	else if (!strDefTexPath.empty())
	{
		size_t i = strPath.find_last_of("/\\");
		if (i != string::npos) strPath = strPath.substr(i + 1);


		if (strDefTexPath.back() == '/' || strDefTexPath.back() == '\\')
			strPath = strDefTexPath + strPath;
		else
			strPath = strDefTexPath + "/" + strPath; 
	}

	// generate IL image id
	C3dglBitmap bm;
	if (bm.load(strPath, GL_RGBA))
	{
		// generate texture id
		glGenTextures(1, &m_idTexture);

		// load texture
		glBindTexture(GL_TEXTURE_2D, m_idTexture); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.getWidth(), bm.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.getBits()); 
	}
}

void C3dglModel::MATERIAL::loadBlankTexture()
{
	if (c_idTexBlank == 0xffffffff)
	{
		glGenTextures(1, &c_idTexBlank);
		glBindTexture(GL_TEXTURE_2D, c_idTexBlank);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		unsigned char bytes[] = { 255, 255, 255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);
	}
	m_idTexture = c_idTexBlank;
}

void C3dglModel::create(const aiScene *pScene)
{
	m_pScene = pScene;
	m_meshes.resize(m_pScene->mNumMeshes, MESH(this));
	aiMesh **ppMesh = m_pScene->mMeshes;
	for (MESH &mesh : m_meshes)
		mesh.create(*ppMesh++, m_maskEnabledBufData);

	m_GlobalInverseTransform = m_pScene->mRootNode->mTransformation;
	m_GlobalInverseTransform.Inverse();
}

void C3dglModel::loadMaterials(const char* pTexRootPath)
{
	if (!m_pScene) return;

	m_materials.resize(m_pScene->mNumMaterials, MATERIAL(this));
	aiMaterial **ppMaterial = m_pScene->mMaterials;
	for (MATERIAL &material : m_materials)
		material.create(*ppMaterial++, pTexRootPath);
}

void C3dglModel::destroy()
{
	if (m_pScene) 
	{
		for (MESH mesh : m_meshes)
			mesh.destroy();
		for (MATERIAL mat : m_materials)
			mat.destroy();
		aiReleaseImport(m_pScene);
		m_pScene = NULL;
	}
}

void C3dglModel::enableBufData(ATTRIB_STD bufId, bool bEnable)
{
	if (bEnable)
		m_maskEnabledBufData |= (1 << bufId);
	else
		m_maskEnabledBufData &= ~(1 << bufId);
}

void C3dglModel::renderNode(aiNode *pNode, glm::mat4 m)
{
	aiMatrix4x4 mx = pNode->mTransformation;;
	aiTransposeMatrix4(&mx);
	m *= glm::make_mat4((GLfloat*)&mx);

	// check if a shading program is active
	C3dglProgram *pProgram = C3dglProgram::GetCurrentProgram();
	
	// send model view matrix
	if (pProgram)
		pProgram->SendStandardUniform(C3dglProgram::UNI_MODELVIEW, m);
	else
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf((GLfloat*)&m);
	}

	for (unsigned iMesh : vector<unsigned>(pNode->mMeshes, pNode->mMeshes + pNode->mNumMeshes))
	{
		MESH *pMesh = &m_meshes[iMesh];
		MATERIAL *pMaterial = pMesh->getMaterial();
		if (pMaterial) pMaterial->bind();
		pMesh->render();
	}

	// draw all children
	for (aiNode *p : vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
		renderNode(p, m);
}

void C3dglModel::render(glm::mat4 matrix)
{
	if (m_pScene->mRootNode)
		renderNode(m_pScene->mRootNode, matrix);
}

void C3dglModel::render(unsigned iNode, glm::mat4 matrix)
{
	// update transform
	aiMatrix4x4 m = m_pScene->mRootNode->mTransformation;
	aiTransposeMatrix4(&m);
	matrix *= glm::make_mat4((GLfloat*)&m);

	if (m_pScene  && m_pScene->mRootNode && iNode <= m_pScene->mRootNode->mNumChildren)
		renderNode(m_pScene->mRootNode->mChildren[iNode], matrix);
}

void C3dglModel::render()
{
	glm::mat4 m;
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)&m);
	render(m);
}

void C3dglModel::render(unsigned iNode)
{
	glm::mat4 m;
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)&m);
	render(iNode, m);
}

void C3dglModel::getNodeTransform(aiNode *pNode, float pMatrix[16], bool bRecursive)
{
	aiMatrix4x4 m1, m2;
	if (bRecursive && pNode->mParent)
		getNodeTransform(pNode->mParent, (float*)&m1, true);

	m2 = pNode->mTransformation;
	aiTransposeMatrix4(&m2);

	*((aiMatrix4x4*)pMatrix) = m2 * m1;
}

unsigned C3dglModel::getBoneId(std::string boneName)
{
	auto it = m_mapBones.find(boneName);
	if (it == m_mapBones.end())
	{
		unsigned i = m_mapBones.size();
		m_mapBones[boneName] = i;
		return i;
	}
	else
		return it->second;

}

bool C3dglModel::getBBNode(aiNode *pNode, aiVector3D BB[2], aiMatrix4x4* trafo)
{
	aiMatrix4x4 prev = *trafo;
	aiMultiplyMatrix4(trafo, &pNode->mTransformation);

	for (unsigned iMesh : vector<unsigned>(pNode->mMeshes, pNode->mMeshes + pNode->mNumMeshes))
	{
		aiVector3D bb[2];
		memcpy(bb, m_meshes[iMesh].getBB(), sizeof(bb));
		aiTransformVecByMatrix4(&bb[0], trafo);
		aiTransformVecByMatrix4(&bb[1], trafo);
		if (bb[0].x < BB[0].x) BB[0].x = bb[0].x;
		if (bb[0].y < BB[0].y) BB[0].y = bb[0].y;
		if (bb[0].z < BB[0].z) BB[0].z = bb[0].z;
		if (bb[1].x > BB[0].x) BB[1].x = bb[1].x;
		if (bb[1].y > BB[0].y) BB[1].y = bb[1].y;
		if (bb[1].z > BB[0].z) BB[1].z = bb[1].z;
	}

	for (aiNode *pNode : vector<aiNode*>(pNode->mChildren, pNode->mChildren + pNode->mNumChildren))
		getBBNode(pNode, BB, trafo);

	*trafo = prev;
	return true;
}

void C3dglModel::getBB(aiVector3D BB[2])
{
	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	BB[0].x = BB[0].y = BB[0].z =  1e10f;
	BB[1].x = BB[1].y = BB[1].z = -1e10f;
	getBBNode(m_pScene->mRootNode, BB, &trafo);
}

std::string C3dglModel::getName()
{
	if (m_name.empty())
		return "";
	else
		return "Model(" + m_name + ")";
}

//////////////////////////////////////////////////////////////////////////////////////
// Articulated Animation Functions

aiVector3D Interpolate(float AnimationTime, const aiVectorKey *pKeys, unsigned nKeys)
{
	// find a pair of keys to interpolate
	unsigned i = 0;
	while (i < nKeys - 1 && AnimationTime >= (float)pKeys[i + 1].mTime)
		i++;

	// if out of bounds, return the last key
	if (i >= nKeys - 1)
		return pKeys[nKeys - 1].mValue;

	// interpolate
	const aiVector3D& Start = pKeys[i].mValue;
	const aiVector3D& End = pKeys[i + 1].mValue;
	float f = (AnimationTime - (float)pKeys[i].mTime) / ((float)(pKeys[i + 1].mTime - pKeys[i].mTime));
	return Start + f * (End - Start);
}

aiQuaternion Interpolate(float AnimationTime, const aiQuatKey *pKeys, unsigned nKeys)
{
	// find a pair of keys to interpolate
	unsigned i = 0;
	while (i < nKeys - 1 && AnimationTime >= (float)pKeys[i + 1].mTime)
		i++;

	// if out of bounds, return the last key
	if (i >= nKeys - 1)
		return pKeys[nKeys - 1].mValue;

	const aiQuaternion& StartRotationQ = pKeys[i].mValue;
	const aiQuaternion& EndRotationQ   = pKeys[i + 1].mValue;    
	float f = (AnimationTime - (float)pKeys[i].mTime) / ((float)(pKeys[i + 1].mTime - pKeys[i].mTime));
	aiQuaternion q;
	aiQuaternion::Interpolate(q, StartRotationQ, EndRotationQ, f);	// spherical interpolation (SLERP)
	return q.Normalize();
}

void ReadNodeHierarchy(const aiAnimation *pAnimation, float AnimationTime, const aiNode* pNode, const aiMatrix4x4& ParentTransform, vector<pair<string, aiMatrix4x4>>& transforms)
{    
	aiMatrix4x4 transform;
     
	// find animation node
	unsigned i = 0;
	string strNodeName = pNode->mName.data;
	while (i < pAnimation->mNumChannels && string(pAnimation->mChannels[i]->mNodeName.data) != strNodeName)
		i++;
    
	if (i < pAnimation->mNumChannels)
	{
		const aiNodeAnim *pNodeAnim = pAnimation->mChannels[i];

		// Interpolate position, rotation and scaling
		aiVector3D vecTranslate = Interpolate(AnimationTime, pNodeAnim->mPositionKeys, pNodeAnim->mNumPositionKeys);
		aiQuaternion quatRotate = Interpolate(AnimationTime, pNodeAnim->mRotationKeys, pNodeAnim->mNumRotationKeys);
		aiVector3D vecScale =     Interpolate(AnimationTime, pNodeAnim->mScalingKeys, pNodeAnim->mNumScalingKeys);

		// create matrices
		aiMatrix4x4 matTranslate, matScale;
		aiMatrix4x4::Translation(vecTranslate, matTranslate);
		aiMatrix4x4 matRotate = aiMatrix4x4(quatRotate.GetMatrix());
		aiMatrix4x4::Scaling(vecScale, matScale);
        
		// Combine the above transformations
		transform = ParentTransform * matTranslate * matRotate * matScale;
	}
	else
		transform = ParentTransform * pNode->mTransformation;

	transforms.push_back(pair<string, aiMatrix4x4>(strNodeName, transform));

	for (unsigned i = 0 ; i < pNode->mNumChildren ; i++)
		ReadNodeHierarchy(pAnimation, AnimationTime, pNode->mChildren[i], transform, transforms);
}

void C3dglModel::getBoneTransforms(unsigned iAnimation, float time, vector<float>& Transforms)
{
	float fTicksPerSecond = (float)GetScene()->mAnimations[0]->mTicksPerSecond;
	if (fTicksPerSecond == 0) fTicksPerSecond = 25.0f;
	time = fmod(time * fTicksPerSecond, (float)GetScene()->mAnimations[0]->mDuration);

	vector<pair<string, aiMatrix4x4>> pairs;
	aiMatrix4x4 transform;
	ReadNodeHierarchy(GetScene()->mAnimations[iAnimation], time, GetScene()->mRootNode, transform, pairs);

	Transforms.resize(m_offsetBones.size() * 16);	// 16 floats per bone matrix

	for (pair<string, aiMatrix4x4> pair : pairs)
	{
		unsigned iBone = getBoneId(pair.first);
		if (iBone < m_offsetBones.size())
		{
			aiMatrix4x4 m = (m_GlobalInverseTransform * pair.second * m_offsetBones[iBone]).Transpose();
			memcpy(&Transforms[iBone * 16], &m, sizeof(m));
		}
	}
}

