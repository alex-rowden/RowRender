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
<dl>
	<dt>Number of Routers</dt>
	<dd>The number of routers to be used in the Nearest Router Tool</dd>
	<dt>Nearest Routers</dt>
	<dd>Button used to run the Nearest Router Routine. Displays the number of routers set above. These routers can be filtered using the "Wifinames" and "Frequencies" submenues.</dd>
	<dt>Total Lights</dt>
	<dd>The number of Virtual Lights to be placed in the virtual environment, this number will be culled before rendering. This effectively sets the light density.</dd>
	<dt>Lights Shown</dt>
	<dd>The number of lights rendered on screen after distance based culling</dd>
	<dt>Constant, Linear, and Quadratic</dt>
	<dd>Lighting parameters</dd>
	<dt>BillBoard Scale</dt>
	<dd>Relative Size of the Quad used to display wifi signal information in place</dd>
	<dt>Shade Instances</dt>
	<dd>Render spheres at the sample locations of the currently selected routers. These spheres are sized proportionately to signal strength.</dd>
	<dt>Line Integral Convolution</dt>
	<dd>Render a Line Integral Convolution plot for the router on the walls. Note: This is only currently supported for 1 router. If off, The wall visualization renders contour lines on the walls. This visualization technique works for Multiple Routers</dd>
	<dt>Alpha Boost</dt>
	<dd>Controls the opacity of the Line Integral Convolution</dd>
	<dt>Screenspace LIC</dt>
	<dd>Used to turn on and off the rendering technique. Slower technique but allows for continuous lines.</dd>
	<dt>Tunable</dt>
	<dd>Temorary Parameter, used currently to modify the threshold of depth discontinuity used to stop screen space LIC.</dd>
	<dt>Hug Walls</dt>
	<dd>Experimental parameter used to remove some aliasing artifacts of Screen Based LIC. Currently, nonfuctional</dd>
	<dt>Procedural Noise</dt>
	<dd>Used to change noise techniques. When active, a procedural noise function is used. When deactivated, a prerendered noise texture is used.
	<dt>Density</dt>
	<dd>Density used in procedural noise generation</dd>
	<dt>num_samples</dt>
	<dd>Number of Samples used in prerendered noise texture</dd>
	<dt>Fragment Position Scale</dt>
	<dd>Used to scale the noise for Line Integral Convolution</dd>
	<dt>Rate</dt>
	<dd>Distance used on each step of the line integral convolution</dd>
	<dt>Use LIC Mask</dt>
	<dd>Mask used in Line Integral Convolution to decrease aliasing artifacts.</dd>
	<dt>frequency_bands</dt>
	<dd>Experimental visualization which uses patterns on the contour lines to visualize frequency in place (one representation rather than two).
	<dt>Display Names</dt>
	<dd>Toggle to control text rendering on contour bands</dt>
	<dt>Number of Dashes</dt>
	<dd>Number of times the text is rendered on a contour band.</dd>
	<dt>Contour Frequency</dt>
	<dd>Control the logarithmic frequency of contours</dd>
	<dt>Linear Term</dt>
	<dd>Fine tune the frequency of contours with a linear component</dd>
	<dt>Thickness</dt>
	<dd>The width of each contour line in ellipsoidal distance</dd>
	<dt>Invert Color Representaiton</dt>
	<dd>Color the wifi contour lines by frequency rather than by router.</dd>
	<dt>texton_background</dt>
	<dd>Render textons on the wall with the contour lines.</dd>
	<dt>Antialiasing</dt>
	<dd>Toggle which turns on and off Anisotropic blending on the noise and texton textures.</dd>
	<dt>Jittered Colors</dt>
	<dd>Toggle which controls the order of colors assigned to routers. When on, nearby routers are maximally distant in hue.</dd>
	<dt>u stretch, and v stretch</dt>
	<dd>Controls the stretch factor in the primary axes of the texton texture.
	<dt>distance mask</dt>
	<dd>Controls where the nearest point in rendered is. Used to give the viewer space with which to interact with the environment</dd>
</dl>
