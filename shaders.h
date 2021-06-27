#pragma once

static const char* vertex_shader_src =
"#version 120\n"
"uniform mat4 mvp;\n"
"attribute vec3 in_color;\n"
"attribute vec3 pos;\n"
"varying vec3 color;"
"void main() {\n"
"   gl_Position = mvp * vec4(pos, 1.0);\n"
//"   gl_Position = vec4(0.5, 0.5, 0.0, 1.0);\n"
"   color = (mvp * vec4(in_color, 1.0)).xyz;\n"
"   vec3 cam = vec3(0.0, 0.0, -10.0);\n"
"   gl_PointSize = 3.0 * 50.0 / (1.0 + length(cam - pos));\n"
"}\n";

// Faux motion blur (requires GL_POINT_SPRITE)
static const char* fragment_shader_src =
"#version 120\n"
"varying vec3 color;\n"
//"in vec2 gl_PointCoord;\n"
"void main() {\n"
"   vec3 n = normalize(color);\n"
"   vec2 nn = normalize(color.xy);"
"   float vv = 0.1*length(color.xy);"
"   float s = clamp(length(color)*0.0025, 0.0, 1.0);"
"   vec3 c = mix(vec3(1.0, 0.0, 0.0), vec3(1.0), s);"
//"   gl_FragColor = vec4(abs(n.x), abs(n.y), abs(n.z), 1.0);\n"
//"   gl_FragColor = vec4(c, 1.0);\n"
"   float pp = length(gl_PointCoord - vec2(0.5)); \n"
"   vec2 thing = gl_PointCoord - vec2(0.5);\n"
"   float yo = 2.0*max(0.0, abs(dot(thing, nn.yx)) - 0.05);"
"   float ye = 1.0*max(0.0, abs(dot(thing, vec2(-nn.x, nn.y))) - 0.5);"
"   ye = abs(dot(thing, vec2(-nn.x, nn.y)));"
"   float tt = (1.0 - 5.0*yo) - max(1.0, 5.0 - vv)*ye;"
"   gl_FragColor = vec4(abs(n), clamp(tt, 0.0, 1.0));\n"
"}\n";
