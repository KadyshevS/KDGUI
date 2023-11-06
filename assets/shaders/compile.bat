@echo off
fxc PSDefault.hlsl /T ps_4_0 /Fo PSDefault.cso
fxc VSDefault.hlsl /T vs_4_0 /Fo VSDefault.cso
fxc PSTexture.hlsl /T ps_4_0 /Fo PSTexture.cso
fxc VSTexture.hlsl /T vs_4_0 /Fo VSTexture.cso
pause