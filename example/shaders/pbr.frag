layout(location = 0) in vec3 Normal;
layout(location = 1) in vec2 TexCoords;
layout(location = 2) in vec3 WorldPos;
layout(location = 0) out vec4 FragColor;

// IBL 0-2
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// material parameters 3-7
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

#include "ubo.glsl"

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal
// mapping the usual way for performance anyways; I do plan make a note of this
// technique somewhere later in the normal mapping tutorial.
vec3
getNormalFromMap()
{
#ifdef HAS_NORMAL_TEXTURE
  vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
#else
  vec3 tangentNormal = vec3(0, 0, 1);
#endif

  vec3 Q1 = dFdx(WorldPos);
  vec3 Q2 = dFdy(WorldPos);
  vec2 st1 = dFdx(TexCoords);
  vec2 st2 = dFdy(TexCoords);

  vec3 N = normalize(Normal);
  vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
  vec3 B = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);

  return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float
DistributionGGX(vec3 N, vec3 H, float roughness)
{
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}
// ----------------------------------------------------------------------------
float
GeometrySchlickGGX(float NdotV, float roughness)
{
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float nom = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}
// ----------------------------------------------------------------------------
float
GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3
fresnelSchlick(float cosTheta, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3
fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) *
                pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
void
main()
{
  // material properties
#ifdef HAS_ALBEDO_TEXTURE
  vec3 albedo = texture(albedoMap, TexCoords).rgb;
#else
  vec3 albedo = vec3(1, 1, 1);
#endif
  albedo *= Model.color.rgb;

#ifdef HAS_METALLIC_TEXTURE
  float metallic = texture(metallicMap, TexCoords).b;
#else
  float metallic = 1.0;
#endif

#ifdef HAS_ROUGHNESS_TEXTURE
  float roughness = texture(roughnessMap, TexCoords).g;
#else
  float roughness = 1.0;
#endif

#ifdef HAS_AO_TEXTURE
  float ao = texture(aoMap, TexCoords).r;
#else
  float ao = 1.0;
#endif

  // input lighting data
  vec3 N = getNormalFromMap();
  vec3 V = normalize(vec3(Env.camPos) - WorldPos);
  vec3 R = reflect(-V, N);

  // calculate reflectance at normal incidence; if dia-electric (like plastic)
  // use F0 of 0.04 and if it's a metal, use the albedo color as F0 (metallic
  // workflow)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  for (int i = 0; i < 4; ++i) {
    // calculate per-light radiance
    vec3 L = normalize(Env.lightPositions[i].xyz - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(Env.lightPositions[i].xyz - WorldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = Env.lightColors[i].rgb * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) +
                        0.0001; // + 0.0001 to prevent divide by zero
    vec3 specular = numerator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance *
          NdotL; // note that we already multiplied the BRDF by the Fresnel (kS)
                 // so we won't multiply by kS again
  }

  // ambient lighting (we now use IBL as the ambient term)
  vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;

  vec3 irradiance = texture(irradianceMap, N).rgb;
  vec3 diffuse = irradiance * albedo;

  // sample both the pre-filter map and the BRDF lut and combine them together
  // as per the Split-Sum approximation to get the IBL specular part.
  const float MAX_REFLECTION_LOD = 4.0;
  vec3 prefilteredColor =
    textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
  vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
  vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

  vec3 ambient = (kD * diffuse + specular) * ao;

  vec3 color = ambient + Lo;

  // HDR tonemapping
  color = color / (color + vec3(1.0));
  // gamma correct
  color = pow(color, vec3(1.0 / 2.2));

  FragColor = vec4(color, 1.0);
}