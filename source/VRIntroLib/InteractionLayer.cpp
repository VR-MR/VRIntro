#include "stdafx.h"
#include "InteractionLayer.h"

#include "Resource.h"
#include "GLShader.h"
#include "GLShaderLoader.h"
#include "GLController.h"

InteractionLayer::InteractionLayer(const EigenTypes::Vector3f& initialEyePos, const std::string& shaderName) :
  m_Shader(Resource<GLShader>(shaderName)),
  m_EyePos(initialEyePos),
  m_Alpha(0.0f) {

}

void InteractionLayer::DrawSkeletonHands() const {
  m_Shader->Bind();
  const EigenTypes::Vector3f desiredLightPos(0, 1.5, 0.5);
  const EigenTypes::Vector3f lightPos = m_EyeView*desiredLightPos;
  const int lightPosLoc = m_Shader->LocationOfUniform("light_position");
  glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);

  for (size_t i = 0; i < m_SkeletonHands.size(); i++) {
    const SkeletonHand hand = m_SkeletonHands[i];
    float distSq = (hand.center - m_EyePos - m_EyeView.transpose()*EigenTypes::Vector3f(0, 0, -0.3f)).squaredNorm();
    float alpha = m_Alpha*std::min(hand.confidence, 0.006f/(distSq*distSq));

    // Set common properties
    m_Sphere.Material().SetDiffuseLightColor(Color(0.4f, 0.6f, 1.0f, alpha));
    m_Sphere.Material().SetAmbientLightColor(Color(0.4f, 0.6f, 1.0f, alpha));
    m_Sphere.Material().SetAmbientLightingProportion(0.3f);

    m_Cylinder.Material().SetDiffuseLightColor(Color(0.85f, 0.85f, 0.85f, alpha));
    m_Cylinder.Material().SetAmbientLightColor(Color(0.85f, 0.85f, 0.85f, alpha));
    m_Cylinder.Material().SetAmbientLightingProportion(0.3f);

    DrawSkeletonHand(hand, alpha);
  }
  m_Shader->Unbind();
}

void InteractionLayer::DrawSkeletonHand(const SkeletonHand& hand, float alpha) const {
  for (int i = 0; i < 23; i++) {
    DrawSphere(hand.joints[i], m_FingerRadius, alpha);
    DrawCylinder(hand.joints[i], hand.jointConnections[i], 0.7f*m_FingerRadius, alpha);
  }
}

//std::shared_ptr<GLShader> m_Shader;
//mutable RenderState m_Renderer;
//EigenTypes::Vector3f m_EyePos;

void InteractionLayer::DrawCylinder(const EigenTypes::Vector3f& p0, const EigenTypes::Vector3f& p1, float radius, float alpha) const {
  m_Cylinder.SetRadius(static_cast<double>(radius));
  m_Cylinder.Translation() = 0.5*(p0 + p1).cast<double>();

  EigenTypes::Vector3f direction = p1 - p0;
  const float length = direction.norm();
  m_Cylinder.SetHeight(length);
  direction /= length;

  EigenTypes::Vector3f Y = direction;
  EigenTypes::Vector3f X = Y.cross(EigenTypes::Vector3f::UnitZ()).normalized();
  EigenTypes::Vector3f Z = Y.cross(X).normalized();

  EigenTypes::Matrix3x3f basis;
  basis << X, Y, Z;
  m_Cylinder.LinearTransformation() = basis.cast<double>();
  PrimitiveBase::DrawSceneGraph(m_Cylinder, m_Renderer);
}

void InteractionLayer::DrawSphere(const EigenTypes::Vector3f& p0, float radius, float alpha) const {
  m_Sphere.SetRadius(static_cast<double>(radius));
  m_Sphere.Translation() = p0.cast<double>();
  PrimitiveBase::DrawSceneGraph(m_Sphere, m_Renderer);
}