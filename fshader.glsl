#version 410

in vec4 color;
out vec4 fragColor;

//for phong
in  vec3 fN;
in  vec3 fL;
in  vec3 fV;

uniform int PhongFlag;
uniform int GouraudFlag;
uniform int ModifiedPhongFlag;
uniform int TexturingFlag;


uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform vec4 LightPosition;
uniform float Shininess;

uniform mat4 ModelView;
uniform vec4 ColorUpdater;

in  vec2 textureCoord;
uniform sampler2D tex; //texture object


void main()
{
    if(PhongFlag==1){
        // Normalize the input lighting vectors
        vec3 N = normalize(fN);
        vec3 V = normalize(fV);
        vec3 L = normalize(fL);
        
        //How I find R: the direction vector R is calculated as the reflection of L on the surface characterized by the surface normal N
        vec3 R = normalize(reflect(L,N));
        
        vec4 ambient = AmbientProduct;
        
        float Kd = max(dot(L, N), 0.0);
        vec4 diffuse = Kd*DiffuseProduct;
        
        float Ks = pow(max(dot(V, R), 0.0), Shininess);
        vec4 specular = Ks*SpecularProduct;
        
        // discard the specular highlight if the light's behind the vertex
        if( dot(L, N) < 0.0 ) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        }
        
        fragColor = ambient + diffuse + specular;
        fragColor +=ColorUpdater;
        
    }else if(GouraudFlag==1){
        
        fragColor = color;
        
    }else if(ModifiedPhongFlag==1){
        
        // Normalize the input lighting vectors
        vec3 N = normalize(fN);
        vec3 V = normalize(fV);
        vec3 L = normalize(fL);
        
        vec3 H = normalize( L + V );
        
        vec4 ambient = AmbientProduct;
        
        float Kd = max(dot(L, N), 0.0);
        vec4 diffuse = Kd*DiffuseProduct;
        
        float Ks = pow(max(dot(N, H), 0.0), Shininess);
        vec4 specular = Ks*SpecularProduct;
        
        // discard the specular highlight if the light's behind the vertex
        if( dot(L, N) < 0.0 ) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        }
        
        fragColor = ambient + diffuse + specular;
        fragColor +=ColorUpdater;
        
    }else if(TexturingFlag==1){
        
         fragColor = texture(tex, textureCoord );
        
    }else{
        fragColor = color;
    }
    
}

