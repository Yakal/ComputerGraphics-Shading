#version 410

in vec4 vPosition;
out vec4 color;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 ColorUpdater;

//gouraud
in  vec3 vNormal;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform float Shininess;

//phong
out  vec3 fN;
out  vec3 fV;
out  vec3 fL;
uniform vec4 LightPosition;

//texture
in  vec2 vTextureCoord;
out vec2 textureCoord;

//for choosing desires shading type
uniform int GouraudFlag;
uniform int PhongFlag;
uniform int ModifiedPhongFlag;
uniform int TexturingFlag;

void main()
{
    if(PhongFlag==1){
        // Transform vertex position into camera (eye) coordinates
        
        vec3 pos = (ModelView * vPosition).xyz;
        fN = (ModelView * vec4(vNormal, 0.0)).xyz; // normal direction in camera coordinates
        fV = -pos; //viewer direction in camera coordinates
        fL = LightPosition.xyz; // light direction
        
        if( LightPosition.w != 0.0 ) {
            fL = LightPosition.xyz - pos;  //directional light source
        }
        
        gl_Position = Projection * ModelView * vPosition;
        
        
    }else if(GouraudFlag==1){
        // Transform vertex position into camera (eye) coordinates
        vec3 pos = (ModelView * vPosition).xyz;
        
        vec3 L = normalize( LightPosition.xyz - pos ); //light direction
        vec3 V = normalize( -pos ); // viewer direction
        vec3 H = normalize( L + V ); // halfway vector
        
        // Transform vertex normal into camera coordinates
        vec3 N = normalize( ModelView * vec4(vNormal, 0.0) ).xyz;
        
        // Compute terms in the illumination equation
        vec4 ambient = AmbientProduct;
        
        float Kd = max( dot(L, N), 0.0 ); //set diffuse to 0 if light is behind the surface point
        vec4  diffuse = Kd*DiffuseProduct;
        
        float Ks = pow( max(dot(N, H), 0.0), Shininess );
        vec4  specular = Ks * SpecularProduct;
        
        //ignore also specular component if light is behind the surface point
        if( dot(L, N) < 0.0 ) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        }
        
        gl_Position = Projection * ModelView * vPosition;
        
        color = ambient + diffuse + specular;
        color +=ColorUpdater;
        
    }else if(ModifiedPhongFlag==1){
        // Transform vertex position into camera (eye) coordinates
        vec3 pos = (ModelView * vPosition).xyz;
        fN = (ModelView * vec4(vNormal, 0.0)).xyz; // normal direction in camera coordinates
        fV = -pos; //viewer direction in camera coordinates
        fL = LightPosition.xyz; // light direction
        
        if( LightPosition.w != 0.0 ) {
            fL = LightPosition.xyz - pos;  //directional light source
        }
        
        gl_Position = Projection * ModelView * vPosition;
        
    }else if(TexturingFlag==1){
        // Transform vertex position into camera (eye) coordinates
        color = ColorUpdater;
        textureCoord= vTextureCoord;
        gl_Position = Projection * ModelView * vPosition;
        
    }
    else{
        gl_Position = Projection * ModelView * vPosition;
        color = ColorUpdater;
    }
}
