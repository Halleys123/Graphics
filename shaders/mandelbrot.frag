// Author:
// Title:

#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;
    
    float color = 0.0;
    
    float zr = 0.0;
    float zi = 0.0;
    
    float scale = 0.112;
    
    float cr = scale * (uv.x + (u_mouse.x / u_resolution.x) - -2.404);
    float ci = scale * (uv.y + (u_mouse.y / u_resolution.y) - 0.5);
    
    float tempZr;
    
    for(int i = 0;i < 10000; i++) {
        float a = zr * zr; 
        float b = zi * zi; 
        
        if(a + b > 2.0) break;
    	
        tempZr = a - b + cr;
        zi = 2.0 * zr * zi + ci;
    	zr = tempZr;
    }
    
    if(zi * zi + zr * zr < 2.0) {
	    gl_FragColor = vec4(uv, 0.164, 1.0);    
    }
}