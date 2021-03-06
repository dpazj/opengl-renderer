#version 400
layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texture_coordinates;
layout(location = 1) in vec3 normal;

uniform mat4 model, view, projection, lightspacemat;

out vec2 TextureCoordinates;
out vec3 Position, Normal;
//out vec4 PositionLightSpace;

uniform bool reverse_normals;

void main()
{
	
	vec4 h_pos =  vec4(position,1);

	TextureCoordinates = texture_coordinates;
	Position = vec3(model * h_pos);

	if(reverse_normals){ // a slight hack to make sure the outer large cube displays lighting from the 'inside' instead of the default 'outside'.
        Normal = transpose(inverse(mat3(model))) * (-1.0 * normal);
	}
    else{
        Normal = transpose(inverse(mat3(model))) * normal;
	}
	//PositionLightSpace = lightspacemat * vec4(Position,1.0f);

	gl_Position = projection * view * model * h_pos;
}