#include <string.h>
#include <stdlib.h>

#include "glr.h"
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

static inline int isPathSeparator(char c)
{
#ifdef _WIN32
  return c == '/' || c == '\\';
#else
  return c == '/';
#endif
}

static const char *resolveTexturePath(const char *objFile, const char *texName)
{
  size_t objFileLen = strlen(objFile);
  size_t texNameLen = strlen(texName);

  const char *dirEnd = objFile + objFileLen;
  while (dirEnd != objFile && !isPathSeparator(dirEnd[-1]))
  {
    dirEnd--;
  }
  size_t dirLen = dirEnd - objFile;
  size_t texFileLen = dirLen + texNameLen;
  char *texFile = (char *)malloc(texFileLen + 1);
  texFile[texFileLen] = '\0';
  memcpy(texFile, objFile, dirLen);
  memcpy(texFile + dirLen, texName, texNameLen);
  return texFile;
}

static void loadFile(void *ctx, const char *filename, const int is_mtl, const char *obj_filename, char **buffer, size_t *len)
{
  GLsizei glrLen = 0;
  *buffer = glrReadFile(filename, "r", &glrLen);
  *len = (size_t)glrLen;
}

GlrModel *glrLoadModel(char *filename, GlrLoadTextureCallback loadTexture)
{
  tinyobj_shape_t *shapes = NULL;
  tinyobj_material_t *materials = NULL;
  size_t shapesLen = 0, materialsLen = 0;
  tinyobj_attrib_t attrib;
  tinyobj_attrib_init(&attrib);

  int tinyobjResult = tinyobj_parse_obj(&attrib, &shapes, &shapesLen, &materials, &materialsLen, filename, loadFile, NULL, TINYOBJ_FLAG_TRIANGULATE);
  if (tinyobjResult != TINYOBJ_SUCCESS)
  {
    return NULL;
  }

  GlrModel *model = (GlrModel *)malloc(sizeof(GlrModel));
  memset(model, 0, sizeof(GlrModel));

  model->verticesLen = attrib.num_vertices;

  // Save the first v/vt/vn for each v
  tinyobj_vertex_index_t **vMap = (tinyobj_vertex_index_t **)malloc(sizeof(void *) * attrib.num_vertices);
  memset(vMap, 0, sizeof(void *) * attrib.num_vertices);
  for (unsigned int i = 0; i < attrib.num_faces; i++)
  {
    tinyobj_vertex_index_t *face = &attrib.faces[i];
    tinyobj_vertex_index_t *seen = vMap[face->v_idx];
    if (seen == NULL)
    {
      vMap[face->v_idx] = face;
    }
    else if (face->vt_idx != seen->vt_idx || face->vn_idx != seen->vn_idx)
    {
      // Duplicated, allocate a new vertex
      ++model->verticesLen;
    }
  }

  model->vertices = (GlrModelVertex *)malloc(sizeof(GlrModelVertex) * model->verticesLen);
  GLuint nextVertexIndex = attrib.num_vertices;
  model->indicesLen = attrib.num_faces;
  model->indices = (GLuint *)malloc(sizeof(GLuint) * attrib.num_faces);
  for (unsigned int i = 0; i < attrib.num_faces; i++)
  {
    tinyobj_vertex_index_t *face = &attrib.faces[i];
    tinyobj_vertex_index_t *seen = vMap[face->v_idx];
    model->indices[i] = face->v_idx;
    if (face->vt_idx != seen->vt_idx || face->vn_idx != seen->vn_idx)
    {
      model->indices[i] = nextVertexIndex;
      ++nextVertexIndex;
      // Use seen == face as a hint to copy vertex data
      seen = face;
    }
    // Skip if face has been seen and is not the first
    if (seen == face)
    {
      GlrModelVertex *vertex = &model->vertices[model->indices[i]];
      memcpy(vertex->position, &attrib.vertices[face->v_idx * 3], sizeof(float) * 3);
      memcpy(vertex->texCoords, &attrib.texcoords[face->vt_idx * 2], sizeof(float) * 2);
      memcpy(vertex->normal, &attrib.normals[face->vn_idx * 3], sizeof(float) * 3);
    }
  }

  free(vMap);
  vMap = NULL;

  model->batchesLen = 1;
  for (unsigned int i = 1; i < attrib.num_face_num_verts; ++i)
  {
    if (attrib.material_ids[i] != attrib.material_ids[i - 1])
    {
      ++model->batchesLen;
    }
  }
  model->batches = (GlrModelBatch *)malloc(sizeof(GlrModelBatch) * model->batchesLen);
  GLuint batchIndex = 0;
  model->batches[0].indicesOffset = 0;
  model->batches[0].indicesLen = 1;
  model->batches[0].materialIndex = materialsLen > 0 ? attrib.material_ids[0] : -1;
  for (unsigned int i = 1; i < attrib.num_face_num_verts; ++i)
  {
    int materialIndex = attrib.material_ids[i];
    if (materialIndex == attrib.material_ids[i - 1])
    {
      ++model->batches[batchIndex].indicesLen;
    }
    else
    {
      ++batchIndex;
      model->batches[batchIndex].indicesOffset = (void *)(i * sizeof(GLuint));
      model->batches[batchIndex].indicesLen = 1;
      model->batches[batchIndex].materialIndex = materialIndex;
    }
  }

  model->materialsLen = materialsLen;
  model->materials = (GlrModelMaterial *)malloc(sizeof(GlrModelMaterial) * materialsLen);
  for (unsigned int i = 0; i < materialsLen; ++i)
  {
    tinyobj_material_t *material = &materials[i];
    GlrModelMaterial *glrMaterial = &model->materials[i];

    glrMaterial->shininess = material->shininess;

    glGenTextures(1, &glrMaterial->diffuse);
    loadTexture(glrMaterial->diffuse, resolveTexturePath(filename, material->diffuse_texname));
    glGenTextures(1, &glrMaterial->specular);
    loadTexture(glrMaterial->specular, resolveTexturePath(filename, material->specular_texname));
  }

  tinyobj_shapes_free(shapes, shapesLen);
  tinyobj_materials_free(materials, materialsLen);
  tinyobj_attrib_free(&attrib);

  return model;
}

void glrBindModel(GlrModel *model)
{
  glGenBuffers(1, &model->vbo);
  glGenBuffers(1, &model->ebo);
  glGenVertexArrays(1, &model->vao);
  glBindVertexArray(model->vao);
  glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
  glBufferData(GL_ARRAY_BUFFER, model->verticesLen * sizeof(GlrModelVertex), model->vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indicesLen * sizeof(GLuint), model->indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GlrModelVertex), (void *)(offsetof(GlrModelVertex, position)));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GlrModelVertex), (void *)(offsetof(GlrModelVertex, normal)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GlrModelVertex), (void *)(offsetof(GlrModelVertex, texCoords)));
  glEnableVertexAttribArray(2);

  // unbind
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void glrDrawModel(GlrModel *model, GlrModelMaterialUniforms *uniforms)
{
  glBindVertexArray(model->vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->ebo);

  for (unsigned int i = 0; i < model->batchesLen; ++i)
  {
    GlrModelBatch *batch = &model->batches[i];
    if (uniforms != NULL && batch->materialIndex >= 0)
    {
      GlrModelMaterial *material = &model->materials[batch->materialIndex];
      glUniform1f(uniforms->shininess, material->shininess);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, material->diffuse);
      glUniform1i(uniforms->diffuse, 0);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, material->specular);
      glUniform1i(uniforms->specular, 1);
    }
    glDrawElements(GL_TRIANGLES, batch->indicesLen, GL_UNSIGNED_INT, batch->indicesOffset);
  }
}

/**
 * @brief Free the resources allocated for the model
 */
void glrFreeModel(GlrModel *model)
{
  free(model->vertices);
  free(model->indices);
  free(model->materials);
  free(model->batches);
  free(model);
}
