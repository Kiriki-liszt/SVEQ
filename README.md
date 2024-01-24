# SVEQ  

<img src="https://raw.githubusercontent.com/Kiriki-liszt/SVEQ/main/screenshot.png"  width="600"/>  

SVEQ is 'State Space', SVF EQ.  
Runs in double precision 64-bit internal processing. Also double precision input / output if supported.  
Internal sample rate is oversampled to 192/176.2kHz in 48/44.1kHz, 96/88.1kHz sample rates.  
It does run under 44.1kHz, but may have some EQ curve cramping.  

## Licensing  

> Q: I would like to share the source code of my VST 3 plug-in/host on GitHub or other such platform.  
>
> * You can choose the GPLv3 license and feel free to share your plug-ins/host's source code including or referencing the VST 3 SDK's sources on GitHub.  
> * **You are allowed to provide a binary form of your plug-ins/host too, provided that you provide its source code as GPLv3 too.**  
> * Note that you have to follow the Steinberg VST usage guidelines.  
>  
> <https://steinbergmedia.github.io/vst3_dev_portal/pages/FAQ/Licensing.html>  

![120C7E3245DD5916ACD2E8E6AD51E8FD_snapshot](https://github.com/Kiriki-liszt/Sky_Blue_EQ4/assets/107096260/142e3c12-cd5f-415d-9b72-8b4f04419633)  

VSTSDK 3.7.9 used  
VSTGUI 4.12 used  

## Version logs

v0.0.1   : intial try.  

## libs  

> git submodule init  
> cd libs  
> git submodule add -f <https://github.com/simd-everywhere/simde.git>  
