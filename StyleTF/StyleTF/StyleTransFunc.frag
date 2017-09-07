#version 330

layout(location = 0) out vec4 FragColor;

in vec3 facePos;

uniform sampler3D volume;
uniform sampler1D transferFunc;
uniform vec3 camPos;

uniform int maxRaySteps;
uniform float rayStepSize;
uniform float gradientStepSize;

uniform vec3 lightPosition;

uniform sampler2DArray styleTextures;
uniform float seperate[3];

vec3 CalculateNormal(vec3 texCoord)
{
	vec3 sample1, sample2;

	sample1.x = texture(volume, texCoord - vec3(gradientStepSize, 0.0f, 0.0f)).x;
	sample2.x = texture(volume, texCoord + vec3(gradientStepSize, 0.0f, 0.0f)).x;
	sample1.y = texture(volume, texCoord - vec3(0.0f, gradientStepSize, 0.0f)).x;
	sample2.y = texture(volume, texCoord + vec3(0.0f, gradientStepSize, 0.0f)).x;
	sample1.z = texture(volume, texCoord - vec3(0.0f, 0.0f, gradientStepSize)).x;
	sample2.z = texture(volume, texCoord + vec3(0.0f, 0.0f, gradientStepSize)).x;

	return normalize(sample2 - sample1);
}


vec4 CalculateLighting(vec4 color, vec3 N)
{
	vec3 lightDirection = normalize(facePos - lightPosition);
	vec4 diffuseLight = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4 specularLight = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	vec3 L, H;

	L = normalize(lightDirection);
	H = normalize(L + normalize(-facePos));
	
	float diff = clamp(dot(N,L), 0.0f, 1.0f);
	float amb = 0.4f;

	return ((color * diff) + (color * amb));
}



float silhouette(vec3 texCoord, vec3 view) 
{
	vec3 sample1, sample2;
	sample1.x = texture(volume, texCoord - vec3(gradientStepSize, 0.0f, 0.0f)).x;
	sample2.x = texture(volume, texCoord + vec3(gradientStepSize, 0.0f, 0.0f)).x;
	sample1.y = texture(volume, texCoord - vec3(0.0f, gradientStepSize, 0.0f)).x;
	sample2.y = texture(volume, texCoord + vec3(0.0f, gradientStepSize, 0.0f)).x;
	sample1.z = texture(volume, texCoord - vec3(0.0f, 0.0f, gradientStepSize)).x;
	sample2.z = texture(volume, texCoord + vec3(0.0f, 0.0f, gradientStepSize)).x;

	vec3 n = normalize(sample2 - sample1);
	float grad = length(sample2 - sample1) / gradientStepSize / 2.;

	float s1 = pow( (1. - abs(dot(n, view))), 3.); // silhouette
	float s2;
	float a = 0.5;
	float b = 10;
	if (grad < a) s2 = 0;
	else if (grad > b) s2 = 1;
	else s2 = (grad-a)/b;

	return s1*s2;
}

vec3 gradient(vec3 texCoord)
{
	vec3 sample1, sample2;
	sample1.x = texture(volume, texCoord - vec3(gradientStepSize, 0.0f, 0.0f)).x;
	sample2.x = texture(volume, texCoord + vec3(gradientStepSize, 0.0f, 0.0f)).x;
	sample1.y = texture(volume, texCoord - vec3(0.0f, gradientStepSize, 0.0f)).x;
	sample2.y = texture(volume, texCoord + vec3(0.0f, gradientStepSize, 0.0f)).x;
	sample1.z = texture(volume, texCoord - vec3(0.0f, 0.0f, gradientStepSize)).x;
	sample2.z = texture(volume, texCoord + vec3(0.0f, 0.0f, gradientStepSize)).x;
	
	return (sample2 - sample1) / gradientStepSize / 2; // gradient
}

float gettex(vec3 texCoord, vec3 d) {
	d = d * gradientStepSize;
	return texture(volume, texCoord + d).x;
}
	

vec4 LoadStyle(vec3 texCoord, vec3 view) 
{
	float index = texture(volume, texCoord).x;

	float alpha = texture(transferFunc, index).w;  // lookup the transfer function


	vec3 grad = CalculateNormal(texCoord);

	vec4 color;
	
	vec3 x_, y_;
	x_ = normalize(vec3(-view.y, view.x, 0));
	y_ = normalize(cross(view, x_));

	vec2 styleCoord = (vec2(dot(grad, x_), dot(grad,y_)) + vec2(1.f)) / 2.f;

	if (index < seperate[0]) {
		color = texture(styleTextures, vec3(styleCoord.x, styleCoord.y, 0));
	}
	else if (index < seperate[1]) {
		color = texture(styleTextures, vec3(styleCoord.x, styleCoord.y, 1));
	}
	else if (index < seperate[2]) {
		color = texture(styleTextures, vec3(styleCoord.x, styleCoord.y, 2));
	}
	else return vec4(0.f);

	if (alpha == 0) color.a = 0;
	else color.a = 0.3 * color.a;

	return color;
}



void main()
{
	vec4 finalColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vec3 position = facePos;
	vec3 texCoord;
	vec4 color;
	vec3 normal;

	vec3 direction = position - camPos;
	direction = normalize(direction);

	float absorption = 0.0f;
	float opacity;

	for(int i=0; i<maxRaySteps; i++)
	{
		color = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		
		texCoord = (position + 1.0f) / 2.0f; 
		color = LoadStyle(texCoord, direction);
		opacity = color.w;

		normal = CalculateNormal(texCoord);
		color = CalculateLighting(color, normal);
			

		if ((absorption + opacity) > 1.0f)
			finalColor += color*(1.0f - absorption);
		else
			finalColor += color*opacity;

		absorption += opacity;

		position = position + (direction * rayStepSize);



		if (abs(position.x) > 1.0f || abs(position.y) > 1.0f || abs(position.z) > 1.0f || absorption >= 0.95f)
		{
		// show a proxy cube
			if (absorption < 1.0f)
				finalColor += vec4(0.7f, 0.7f, 0.7f, 1.0f) * (1.0f - absorption);

			break;
		}
	}


	FragColor = finalColor;
}
