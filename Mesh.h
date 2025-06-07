#pragma once  
#include "Model.h"  
//include assimp header files
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Mesh {  
public:  
   Mesh(const char* OBJ_Path, const char* ShaderName)  
       : model(  
           std::string("Shaders/" + std::string(ShaderName) + ".vert").c_str(),  
           std::string("Shaders/" + std::string(ShaderName) + ".frag").c_str(),  
           loadOBJ(OBJ_Path).vertices,  
           loadOBJ(OBJ_Path).indices  
       )  
   {  
   }  

   meshData loadOBJ(const char* OBJ_Path) {
       meshData data;
       // Vertices = COORDINATES vec3, COLORS vec3, TexCoord vec2, NORMALS vec3
       Assimp::Importer importer;
       const aiScene* scene = importer.ReadFile(
           OBJ_Path,
           aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs
       );

       if (!scene || !scene->HasMeshes()) {
           // Handle error: could not load file or no meshes found
           return data;
       }

       const aiMesh* mesh = scene->mMeshes[0];

       for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
           // Position
           data.vertices.push_back(mesh->mVertices[i].x);
           data.vertices.push_back(mesh->mVertices[i].y);
           data.vertices.push_back(mesh->mVertices[i].z);

           // Color (if available, else default to white)
           if (mesh->HasVertexColors(0)) {
               data.vertices.push_back(mesh->mColors[0][i].r);
               data.vertices.push_back(mesh->mColors[0][i].g);
               data.vertices.push_back(mesh->mColors[0][i].b);
           }
           else {
               data.vertices.push_back(1.0f);
               data.vertices.push_back(1.0f);
               data.vertices.push_back(1.0f);
           }

           // Texture Coordinates (if available)
           if (mesh->HasTextureCoords(0)) {
               data.vertices.push_back(mesh->mTextureCoords[0][i].x);
               data.vertices.push_back(mesh->mTextureCoords[0][i].y);
           }
           else {
               data.vertices.push_back(0.0f);
               data.vertices.push_back(0.0f);
           }

           // Normals (if available)
           if (mesh->HasNormals()) {
               data.vertices.push_back(mesh->mNormals[i].x);
               data.vertices.push_back(mesh->mNormals[i].y);
               data.vertices.push_back(mesh->mNormals[i].z);
           }
           else {
               data.vertices.push_back(0.0f);
               data.vertices.push_back(0.0f);
               data.vertices.push_back(0.0f);
           }
       }

       // Indices
       for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
           const aiFace& face = mesh->mFaces[i];
           for (unsigned int j = 0; j < face.mNumIndices; ++j) {
               data.indices.push_back(face.mIndices[j]);
           }
       }

       return data;
   }

   VE::Model model;
private:  
};
