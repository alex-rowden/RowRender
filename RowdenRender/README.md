# RowRender README
## Installation
At the moment there is only one way to access the RowRender, which is through the zipped directory found at https://obj.umiacs.umd.edu/wifivisualizationcode/RowRender.7z. Note due to the fact that this project is still currently in development, the file is quite large (1.3Gb compressed, 9Gb uncompressed at the time of writing). 
In order to preview the visualizations, you must unzip the directory onto your machine. Then navigate to "./RowRender/RowdenRender/" in a command prompt. From there you can view any of the visualizations by running, 

- To view the indoor visualization run "..\x64\Release\RowRender.exe"
- To view the campus visualization run the command  "..\x64\Release\RowRender.exe" campus
- To view either visualization in vr also add the command "vr" (Please note that as of writing this readme vr is not supported in the campus visualization but that should be rectified soon).

## Use
### Indoor Visualization
The Indoor Visualization has many different settings in the Gui. Here is a rundown on how each of them function.
 
Number of Routers
: The number of routers to be used in the Nearest Router Tool

Nearest Routers
: Button used to run the Nearest Router Routine. Displays the number of routers set above. These routers can be filtered using the "Wifinames" and "Frequencies" submenues.

Total Lights
: The number of Virutal Lights to be placed in the virtual environment, this number will be culled before rendering. This effectively sets the light density.

