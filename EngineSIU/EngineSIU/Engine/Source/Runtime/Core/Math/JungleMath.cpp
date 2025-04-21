#include "Math/JungleMath.h"
#include <DirectXMath.h>
#include "MathUtility.h"

#include "Rotator.h"

using namespace DirectX;


FVector4 JungleMath::ConvertV3ToV4(FVector vec3)
{
	FVector4 newVec4;
	newVec4.X = vec3.X;
	newVec4.Y = vec3.Y;
	newVec4.Z = vec3.Z;
	return newVec4;
}

FMatrix JungleMath::CreateModelMatrix(FVector translation, FVector rotation, FVector scale)
{
    FMatrix Translation = FMatrix::CreateTranslationMatrix(translation);

    FMatrix Rotation = FMatrix::CreateRotationMatrix(rotation.X, rotation.Y, rotation.Z);
    //FMatrix Rotation = JungleMath::EulerToQuaternion(rotation).ToMatrix();

    FMatrix Scale = FMatrix::CreateScaleMatrix(scale.X, scale.Y, scale.Z);
    return Scale * Rotation * Translation;
}

FMatrix JungleMath::CreateModelMatrix(FVector translation, FQuat rotation, FVector scale)
{
    FMatrix Translation = FMatrix::CreateTranslationMatrix(translation);
    FMatrix Rotation = rotation.ToMatrix();
    FMatrix Scale = FMatrix::CreateScaleMatrix(scale.X, scale.Y, scale.Z);
    return Scale * Rotation * Translation;
}
FMatrix JungleMath::CreateViewMatrix(FVector eye, FVector target, FVector up)
{
    FVector zAxis = (target - eye).GetSafeNormal();  // DirectX는 LH이므로 -z가 아니라 +z 사용
    
    // 2) up·zAxis 코사인 값으로 평행도 체크
    const float parallelThreshold = 0.999f;
    float dotUZ = FVector::DotProduct(up, zAxis);

    // FIXME : 적용 제대로 안됨
    // 너무 평행하면 다른 업벡터를 시도
    //if (FMath::Abs(dotUZ) > parallelThreshold)
    //{
    //    // worldUp 후보 1
    //    up = FVector(1, 0, 0);
    //    dotUZ = FVector::DotProduct(up, zAxis);
    //    if (FMath::Abs(dotUZ) > parallelThreshold)
    //    {
    //        // worldUp 후보 2
    //        up = FVector(0, 1, 0);
    //        dotUZ = FVector::DotProduct(up, zAxis);
    //        if (FMath::Abs(dotUZ) > parallelThreshold) {
    //            up = FVector(0, 1, 1);
    //        }
    //    }
    //}

    FVector xAxis = (up.Cross(zAxis)).GetSafeNormal();
    FVector yAxis = zAxis.Cross(xAxis);

    FMatrix View;
    View.M[0][0] = xAxis.X; View.M[0][1] = yAxis.X; View.M[0][2] = zAxis.X; View.M[0][3] = 0;
    View.M[1][0] = xAxis.Y; View.M[1][1] = yAxis.Y; View.M[1][2] = zAxis.Y; View.M[1][3] = 0;
    View.M[2][0] = xAxis.Z; View.M[2][1] = yAxis.Z; View.M[2][2] = zAxis.Z; View.M[2][3] = 0;
    View.M[3][0] = -xAxis.Dot(eye);
    View.M[3][1] = -yAxis.Dot(eye);
    View.M[3][2] = -zAxis.Dot(eye);
    View.M[3][3] = 1;

    return View;
}

FMatrix JungleMath::CreateProjectionMatrix(float fov, float aspect, float nearPlane, float farPlane)
{
    float tanHalfFOV = tan(fov / 2.0f);
    float depth = farPlane - nearPlane;

    FMatrix Projection = {};
    Projection.M[0][0] = 1.0f / (aspect * tanHalfFOV);
    Projection.M[1][1] = 1.0f / tanHalfFOV;
    Projection.M[2][2] = farPlane / depth;
    Projection.M[2][3] = 1.0f;
    Projection.M[3][2] = -(nearPlane * farPlane) / depth;
    Projection.M[3][3] = 0.0f;  

    return Projection;
}

FMatrix JungleMath::CreateOrthoProjectionMatrix(float width, float height, float nearPlane, float farPlane)
{
    float r = width * 0.5f;
    float t = height * 0.5f;
    float invDepth = 1.0f / (farPlane - nearPlane);

    FMatrix Projection = {};
    Projection.M[0][0] = 1.0f / r;
    Projection.M[1][1] = 1.0f / t;
    Projection.M[2][2] = invDepth;
    Projection.M[3][2] = -nearPlane * invDepth;
    Projection.M[3][3] = 1.0f;

    return Projection;
}

FVector JungleMath::FVectorRotate(FVector& origin, const FVector& InRotation)
{
    FQuat quaternion = JungleMath::EulerToQuaternion(InRotation);
    // 쿼터니언을 이용해 벡터 회전 적용
    return quaternion.RotateVector(origin);
}
FQuat JungleMath::EulerToQuaternion(const FVector& eulerDegrees)
{
    float yaw = FMath::DegreesToRadians(eulerDegrees.Z);   // Z축 Yaw
    float pitch = FMath::DegreesToRadians(eulerDegrees.Y); // Y축 Pitch
    float roll = FMath::DegreesToRadians(eulerDegrees.X);  // X축 Roll

    float halfYaw = yaw * 0.5f;
    float halfPitch = pitch * 0.5f;
    float halfRoll = roll * 0.5f;

    float cosYaw = cos(halfYaw);
    float sinYaw = sin(halfYaw);
    float cosPitch = cos(halfPitch);
    float sinPitch = sin(halfPitch);
    float cosRoll = cos(halfRoll);
    float sinRoll = sin(halfRoll);

    FQuat quat;
    quat.W = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
    quat.X = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
    quat.Y = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
    quat.Z = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;

    quat.Normalize();
    return quat;
}
FVector JungleMath::QuaternionToEuler(const FQuat& quat)
{
    FVector euler;

    // 쿼터니언 정규화
    FQuat q = quat;
    q.Normalize();

    // Yaw (Z 축 회전)
    float sinYaw = 2.0f * (q.W * q.Z + q.X * q.Y);
    float cosYaw = 1.0f - 2.0f * (q.Y * q.Y + q.Z * q.Z);
    euler.Z = FMath::RadiansToDegrees(atan2(sinYaw, cosYaw));

    // Pitch (Y 축 회전, 짐벌락 방지)
    float sinPitch = 2.0f * (q.W * q.Y - q.Z * q.X);
    if (fabs(sinPitch) >= 1.0f)
    {
        euler.Y = FMath::RadiansToDegrees(copysign(PI / 2, sinPitch)); // 🔥 Gimbal Lock 방지
    }
    else
    {
        euler.Y = FMath::RadiansToDegrees(asin(sinPitch));
    }

    // Roll (X 축 회전)
    float sinRoll = 2.0f * (q.W * q.X + q.Y * q.Z);
    float cosRoll = 1.0f - 2.0f * (q.X * q.X + q.Y * q.Y);
    euler.X = FMath::RadiansToDegrees(atan2(sinRoll, cosRoll));
    return euler;
}
FVector JungleMath::FVectorRotate(FVector& origin, const FRotator& InRotation)
{
    return InRotation.ToQuaternion().RotateVector(origin);
}

FMatrix JungleMath::CreateRotationMatrix(FVector rotation)
{
    XMVECTOR quatX = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), FMath::DegreesToRadians(rotation.X));
    XMVECTOR quatY = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), FMath::DegreesToRadians(rotation.Y));
    XMVECTOR quatZ = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), FMath::DegreesToRadians(rotation.Z));

    XMVECTOR rotationQuat = XMQuaternionMultiply(quatZ, XMQuaternionMultiply(quatY, quatX));
    rotationQuat = XMQuaternionNormalize(rotationQuat);  // 정규화 필수

    XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotationQuat);
    FMatrix result = FMatrix::Identity;  // 기본값 설정 (단위 행렬)

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.M[i][j] = rotationMatrix.r[i].m128_f32[j];  // XMMATRIX에서 FMatrix로 값 복사
        }
    }
    return result;
}
