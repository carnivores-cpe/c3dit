# C3Dit - Carnivores Character 3D Editor
**Version: 2.0.0**  
**Date: DD MM 2020**  
**A Carnivores-CPE project**  
**Home:** [GitHub](https://github.com/carnivores-cpe/c3dit)

----
## Disclaimer
C3Dit is an experimental modding tool in heavy development, we take no responsibility for any damage it may or may not cause to your game files, make backups of your files before you use the tool.

----
## License
> LICENSE.md

----
## Changelog
2.0.0

* Complete rework of the program's internal functionality.
* Uniformity of popup dialog windows to keep things precise and clean
* Settings dialog for key bindings and general settings
* Separation of the Win32 API from the main editor functionality (Cleaner looking and easier to edit code for contributors)
* Increased stability
* Support for extremely large amounts of faces and vertices.
* Lot's of case checking to prevent corruption of output files.
* Native support for the following file types: `CAR 3DF C2O 3DN OBJ TGA BMP WAV VTL`
* OpenGL 2.1 Renderer
* Direct3D 9.0 Renderer

----
## Main
Using an external modelling utility such as Blender, AutoDesk 3D Studio Max, AutoDesk GMax and so on, you now have the capability to create your own Carnivores compatible model formats for creatures, weapons, scenery and more!

C3Dit is a utility specifically designed to work with the AtmosFear 2 engine file formats, taking aspects from the game code in order to provide you as the end user, the most accurate and useful utility yet.
Specifically it is made to produce functional .CAR files for use with the game, it can also export .3DF files.

----
## Instructions
Unzip the application into some preferred location such as "Documents\Carnivores Modding\C3Dit\"
Open the directory in which you unzipped C3Dit, click on the `C3Dit.exe` executable to run the application.
Viola installation complete.

If for some reason an application setting is causing C3Dit to crash, you can delete the `C3Dit.ini` file, a default one will be automatically generated the next time C3Dit is started.

Errors are logged to `error.log` and may be attached to any bug reports.

----
## Credits
### Code Contributors
* Rexhunter99

### Research
* Machf
* Dinoguy1000
