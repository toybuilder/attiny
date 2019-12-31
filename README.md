# attiny
 Collection of maybe useful stuff for ATTiny

nvm/  
 Routines and test code to read/write from the onboard flash.
 The test code demonstate important concepts about the use of the flash and ATTiny quirks.
 You must set the SELFPROGEN feature in the fuse or the flash routines won't work at all.
 Flash writes are done a word at a time (2 bytes); and each page of Flash is 8 words
 (16 bytes) long.
 The code largely started out by copying the work from Avrnaut at https://www.avrfreaks.net/forum/attiny104-self-programming
 

