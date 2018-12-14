#version 430 core

#define FAR_CLIP  10000.0f
#define width 1080
#define height 720
struct TexCoords
{
	vec4 A, B, C;	
};
struct TriNormal
{
	vec4 A, B, C;
};

struct Primitive
{
	vec4 A, B, C;
};
struct Triangle
{
	Primitive p;
	TriNormal n;
	TexCoords t;
};
struct Light{
	vec4	dir;
	vec4	color;
	vec4	attenuation;
};

struct Camera{
	vec3	pos, dir, yAxis, xAxis ;
	float	tanFovY, tanFovX;
};
struct Ray{
	vec3	origin;
	vec3	dir;
};

struct Material
{
	vec4 diffuse;
	vec4 specular;
	vec4 emission;
	float shininess;	
};


float hitTriangle(Ray r, Primitive t);
float hitSphere(Ray r, Primitive s);

layout(binding = 0, rgba32f) uniform image2D framebuffer;
layout(binding = 1) buffer meshtriangles
{
	Triangle tris[ ];	
};
layout(binding = 2) buffer sphereBuff
{
	Triangle sphere;	
};

uniform int mNumTriangles;

uniform Material materials[2];


uniform uint reflectionDepth;

uniform Camera camera;


uniform Light lights[4];
uniform int numLights;


Ray getReflectionRay(Ray r, int currentObject, float t)
{
int id = 0;

if(currentObject == -2)
id = 1;
	vec3 hitPoint = r.origin + r.dir * t;
	vec3 N = vec3(0,0,0);
	
			if(id == 0)
			{
			N = normalize(cross((tris[currentObject].p.B - tris[currentObject].p.A).xyz, (tris[currentObject].p.C - tris[currentObject].p.A).xyz) / 3);	
			}
			else
			{
			N = normalize(hitPoint - sphere.p.A.xyz);
			}
				
	
	
	
	vec3 dir = normalize( r.dir - 2 * dot(r.dir, N) * N);
	Ray ray = { hitPoint+dir*0.01f, dir};
	
	return ray;
}




vec4 calculateColor(Ray r, float t, int currentObject){
	vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	int id = 0;

	//Saying its a sphere.
	if(currentObject == -2)
	id = 1;


	if(currentObject != -1)
	{
		vec3 hitPoint = r.origin + t*r.dir;
		vec3 N, L, H, attCoef;
		Ray shadowRay;
		bool inShadow = false;
		bool lightet = false;
		float temp = FAR_CLIP;
		int	x = -1, lightType = 1;
		
			if(id == 0)
			{
			N =  normalize(cross((tris[currentObject].p.B - tris[currentObject].p.A).xyz, (tris[currentObject].p.C - tris[currentObject].p.A).xyz));	
			}
			else
			{
			N = normalize(hitPoint - sphere.p.A.xyz);
			}
	

			hitPoint += 0.01f*N;
		
	
			inShadow = false;
			for(int j = 0; j < numLights; ++j)
			{

			inShadow = false;
			
			lightType = int(lights[j].attenuation.w);

			switch(lightType)
			{
				//Point light
				case 1:{
					L = normalize(lights[j].dir.xyz - hitPoint);
				}break;

				//Directional light
				case 2:{
					L = normalize(lights[j].dir.xyz);
				}break;
			}
			

			shadowRay = Ray( hitPoint, L);

			int i = 0;

			temp = hitSphere(shadowRay, sphere.p);

					switch(lightType){
					//Point light
					case 1:{
						if( (temp < FAR_CLIP && temp >= -0.001f && temp < length(hitPoint - lights[j].dir.xyz))){
							inShadow = true;
							i = int(mNumTriangles);
						}
					} break;

					//Directional light
					case 2:{
						if(temp < FAR_CLIP && temp >= -0.001f){
							inShadow = true;
							i = int(mNumTriangles);
						}
					} break;
				}

			for(i = 0; i < mNumTriangles; ++i)
			{
			temp = hitTriangle(shadowRay, tris[i].p);
					
			

				switch(lightType){
					//Point light
					case 1:{
						if( (temp < FAR_CLIP && temp >= -0.001f && temp < length(hitPoint - lights[j].dir.xyz))){
							inShadow = true;
							i = int(mNumTriangles);
						}
					} break;

					//Directional light
					case 2:{
						if(temp < FAR_CLIP && temp >= -0.001f){
							inShadow = true;
							i = int(mNumTriangles);
						}
					} break;
				}
			}


		
			

			if(!inShadow)
			{
			
				H = normalize(L + normalize(camera.pos.xyz-hitPoint));

				if(dot(N, L) > 0)
				{		

				attCoef = lights[j].color.xyz / ( lights[j].attenuation.x +
							  lights[j].attenuation.y * length(lights[j].dir.xyz - hitPoint)  +
							  lights[j].attenuation.z * pow( length(lights[j].dir.xyz - hitPoint)*0.1f , 2) );

					color += vec4(materials[id].diffuse.xyz * max(dot(N, L), 0) + materials[id].specular.xyz * pow( max( dot(-N,H), 0), materials[id].shininess), 0.0f);
				

					if(x != -1){
						color += vec4( attCoef * materials[id].diffuse.xyz * max(dot(N, L), 0) + materials[id].specular.xyz * pow( max( dot(-N,H), 0), materials[id].shininess), 0.0f);
					} else {
						color += vec4(attCoef * (vec3(0.5,0.5,0.5) * max(dot(N, L), 0) + 
									   vec3(0.5,0.5,0.5) * pow( max( dot(N,H), 0), 10)), 
									   0.0f);
					}
				}
				inShadow = false;
				lightet = true;
			}
		}
		

		if(lightet)
		{
			if(x!=-1)
			{
				color += materials[id].emission;
			}
		}
	}
	return color;
}

float hitTriangle(Ray r, Primitive t)
{
	vec3 AB = t.B.xyz - t.A.xyz;
	vec3 AC = t.C.xyz - t.A.xyz;

	float det = determinant( mat3(AB, AC, -1.0f*r.dir) );
	
	if(det == 0.0f){
		return FAR_CLIP;
	} else {
		vec3 oA = r.origin - t.A.xyz;
		
		mat3 Di = inverse(mat3(AB, AC, -1.0f*r.dir));
		vec3 solution = Di*oA;

		if(solution.x >= -0.0001 && solution.x <= 1.0001){
			if(solution.y >= -0.0001 && solution.y <= 1.0001){
				if(solution.x + solution.y <= 1.0001){
					return solution.z;
				}
			}
		}
		return FAR_CLIP;
	}
} 

//ray.dir has to be normalized
float hitSphere(Ray r, Primitive s)
{
	
	vec3 oc = r.origin - s.A.xyz;
	float s_roc = dot(r.dir, oc);
	float s_oc = dot(oc, oc);
	
	float d = s_roc*s_roc - s_oc + s.A.w*s.A.w;

	if(d < 0){
		return FAR_CLIP;
	} else if(d == 0) {
		if(-s_roc < 0){
			return FAR_CLIP;
		}

		return -s_roc;
	} else {
		float t1 = 0, t2 = 0;
		
		t1 = sqrt(d);
		t2 = -s_roc-t1;
		t1 = -s_roc+t1;
		
		//ray origin lies in the sphere
		if( (t1 < 0 && t2 > 0)  || (t1 > 0 && t2 <0)){
			return FAR_CLIP;
		}
		
		if( (t2>t1 ? t1 : t2) < 0){
			return FAR_CLIP;
		} else {
			return (t2>t1 ? t1 : t2);
		}
	}
}

Ray initRay(uint x, uint y, Camera cam){
	Ray r;
	vec3 dir;
	float a, b, halfWidth, halfHeight;
		
	halfWidth = float(width)/2.0f;
	halfHeight = float(height)/2.0f;

	a = cam.tanFovX*( (float(x)-halfWidth+0.5f) / halfWidth);
	b = cam.tanFovY*( (halfHeight - float(y)-0.5f) / halfHeight);

	dir = normalize( a*cam.xAxis.xyz + b*cam.yAxis.xyz + cam.dir.xyz);
		
	r.dir = dir;
	r.origin = cam.pos.xyz;
	
	return r;
}

vec4 handleSphere(uint x, uint y)
{
	if(x < width && y < height)
	{
		vec4 color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		vec4 tempColor = vec4(0);

		//Initialize the ray
		Ray r = initRay(x, y, camera);

		float t = FAR_CLIP, temp = FAR_CLIP;
		int currentObject = 1;



	for(uint n = 0; n < reflectionDepth; ++n)
		{
		
						
				temp = hitSphere(r, sphere.p);
					
					
				
				if(temp < t && temp >= -0.001f)
				{
					t = temp;
					currentObject = -2;
				}
			

			if(currentObject == -2)
			{
				color = calculateColor(r, t, currentObject);
				if(tempColor != vec4(0))
				{
					if(materials[1].specular != vec4(0))
					{
						color += materials[1].specular * tempColor;
						r = getReflectionRay(r, currentObject, t);
						
						currentObject = -1;
						temp = t = FAR_CLIP;
					}
					 else
					 {
						color += tempColor;
						n = reflectionDepth;
					}
				} 
				else 
				{
					n = reflectionDepth;
				}
			} 
			else
			{
				n = reflectionDepth;
			}
		}

		return color;
	}

}
vec4 handleTriangles(uint x, uint y)
{


	if(x < width && y < height)
	{
		vec4 color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		vec4 tempColor = vec4(0);

		//Initialize the ray
		Ray r = initRay(x, y, camera);

		float t = FAR_CLIP, temp = FAR_CLIP;
		int currentObject = -1;


int id = 0;
	for(uint n = 0; n < reflectionDepth; ++n)
		{
			for(int i = 0; i < mNumTriangles; i++)
			{
				
				
				temp = hitTriangle(r, tris[i].p);
					
					
				
				if(temp < t && temp >= -0.001f)
				{
					t = temp;
					currentObject = i;
				}
			}

			temp = hitSphere(r, sphere.p);

			if(temp < t && temp >= -0.001f)
			{
				t = temp;
				currentObject = -2;
				id = 1;
			}

			if(currentObject != -1)
			{
				color = calculateColor(r, t, currentObject);
				if(tempColor != vec4(0))
				{
					if(materials[id].specular != vec4(0))
					{
						color += materials[id].specular * tempColor;
						r = getReflectionRay(r, currentObject, t);
						
						currentObject = -1;
						temp = t = FAR_CLIP;
					}
					 else
					 {
						color += tempColor;
						n = reflectionDepth;
					}
				} 
				else 
				{
					n = reflectionDepth;
				}
			} 
			else
			{
				n = reflectionDepth;
			}
		}

		return color;
	}
}



layout (local_size_x = 25, local_size_y = 8) in;
void main(void) 
{
	uint x = gl_GlobalInvocationID.x;
	uint y = gl_GlobalInvocationID.y;
	vec4 color = vec4(0, 0, 0, 0);

	
	color += handleTriangles(x, y);

	imageStore(framebuffer, ivec2(x, y), color);

	
}