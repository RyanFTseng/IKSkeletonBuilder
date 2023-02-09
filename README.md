# SkeletonBuilder
This skeleton builder allows for interactive skeleton building using joints. Joints can be created and linked together to create a dynamic skeleton. Created joints can be either linked to a parent joint or stand alone as an individual joint. Created skeletons can also be saved as a .txt file to /bin to load into the application using drag-and-drop. </br>
![image](https://user-images.githubusercontent.com/66948916/217885682-e242ed19-0c4c-410b-88bf-2067737dfc32.png)

# IK Solver
The IK Solver in this application uses the FABRIK algorithim. The IK solver can be toggled on/off using a hotkey and creates an interactive target point for a chain of joints to reach. </br>

![image](https://user-images.githubusercontent.com/66948916/217887718-9bf002cf-67fa-4063-b992-4de3bc047e1c.png)
![image](https://user-images.githubusercontent.com/66948916/217887759-cd8d686a-ac87-4590-bd21-2831ed14c680.png)



# KeyFraming
Keyframe animation is simulated using a target point from the IK Solver to save positions for a chain of joints. Keyframes are simulated using procedural animation. </br>


# Controls
Skeleton Builder </br>
J - Create New Joint </br>
DEL - Delete Selected Joint </br>
Hold X/Y/Z + Drag - Rotate Joint on X/Y/Z-Axis</br>
S- Save Skeleton</br>
I - Toggle IK Solver</br>


Animation </br>
K - Save KeyFrame</br>
D - Clear Keyframes</br>
P - Begin Playback</br>

# Run
Run Skele.exe in /bin
