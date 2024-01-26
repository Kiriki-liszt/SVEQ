# SVEQ  

<img src="https://raw.githubusercontent.com/Kiriki-liszt/SVEQ/main/screenshot.png"  width="600"/>  

SVEQ is 'State Space' modeling, clean surgical EQ.  
Runs in double precision 64-bit internal processing. Also double precision input / output if supported.  
Internal sample rate is oversampled to 192/176.2kHz in 48/44.1kHz, 96/88.1kHz sample rates.  
It does run under 44.1kHz, but may have some EQ curve cramping.  

Windows and Mac, VST3 and AU.  

## How to use  

1. Windows

Unzip Win.zip from latest release and copy to "C:\Program Files\Common Files\VST3".  

2. MacOS(Intel tested, Apple Silicon not tested).  

Unzip MacOS.zip from latest release and copy vst3 to "/Library/Audio/Plug-Ins/VST3" and component to "/Library/Audio/Plug-Ins/Components".  

> If it doesn't go well, configure security options in console as  
>  
> ``` console  
> sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/VST3/SVEQ.vst3  
> sudo xattr -r -d com.apple.quarantine /Library/Audio/Plug-Ins/Components/SVEQ.component
>
> sudo codesign --force --sign - /Library/Audio/Plug-Ins/VST3/SVEQ.vst3  
> sudo codesign --force --sign - /Library/Audio/Plug-Ins/Components/SVEQ.component
> ```  
>  
> tested by @jonasborneland [here](https://github.com/Kiriki-liszt/JS_Inflator_to_VST2_VST3/issues/12#issuecomment-1616671177)


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

## Fonts  

Font used in this plugin is 'Pretendard' by 길형진.  
[https://github.com/orioncactus/pretendard](https://github.com/orioncactus/pretendard)  
The font is under OFL-1.1 license.  
Modified for monospace number.  

## Version logs

v0.0.1   : intial try.  
v1.0.0   : stable release.  

## libs  

> git submodule init  
> cd libs  
> git submodule add -f <https://github.com/simd-everywhere/simde.git>  
