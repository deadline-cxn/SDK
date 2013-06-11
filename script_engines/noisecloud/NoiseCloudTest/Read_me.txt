NoiseCloudTest by Kim Pallister
Copyright © 2000 Intel Corporation
All Rights Reserved

License
-------

Permission is granted to use, copy, distribute and prepare derivative works of this 
software for any purpose and without fee, provided, that the above copyright notice
and this statement appear in all copies.  Intel makes no representations about the
suitability of this software for any purpose.  This software is provided "AS IS." 

Intel specifically disclaims all warranties, express or implied, and all liability,
including consequential and other indirect damages, for the use of this software, 
including liability for infringement of any proprietary rights, and including the 
warranties of merchantability and fitness for a particular purpose.  Intel does not
assume any responsibility for any errors which may appear in this software nor any
responsibility to update it.


Description
-----------

This is a demo of procedural noise clouds, using DX7. Render to 
texture and bilerping are used do most of the steps in generating 
the clouds.

The demo has been tested on the following chipsets:

- Matrox G400
- Nvidia GeForce*
- Nvidia TNT2* 

* Seems to have issues in 16bpp though. Run it in 32bpp or in one of 
the full screen 32bpp modes)

Demo currently does not work on:

- ATI Rage 128 (should work, couldn't figure out what the issue was here)
- 3Dfx Voodoo 1/2 (these don't support render to texture)

The demo has not been tested on:

S3 Savage 3 (though it should work here)
Nvidia TNT
3Dfx Voodoo 3
Matrox G200 (should work on this as well)

Comments and questions can be sent to kim.pallister@intel.com
