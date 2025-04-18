#include "AmbientLightComponent.h"

#include "UObject/Casts.h"

UAmbientLightComponent::UAmbientLightComponent()
{
    LightColor = FColor(25, 25, 25, 255);
}
