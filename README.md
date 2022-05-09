# Image-processing-using-open-MPI
This is a short parallel program that processes a blurry grey scale image into a clearer one by increasing its contrast.
Image processing tools provide a large range of different algorithms for all kind of circumstances. A simple and effective algorithm is the Histogram equalization. It re-adjusts the contrast of an image using the image's histogram.
<p float="left">
<img src="https://user-images.githubusercontent.com/45368931/167435661-4a806568-0674-4026-b868-98a3b90f60c8.jpg" title="Before processing" width="45%"> 
&nbsp; &nbsp; &nbsp; &nbsp;
<img src="https://user-images.githubusercontent.com/45368931/167435932-994c7f03-923a-4e8f-9f4f-a62512173835.jpg" title="After processing"  width="45%"> 
</p>
Foster's methodology is applied to parallelise the program: It involves four main steps.<br /><br />
1.<b>Partitioning</b>:
Here, we decide which tasks and data will be divided among the processes.
Data: The image, each process will get a bunch of rows. For dimensions that are not completely divisible by number of processes, the remainder is added onto the chunk for the last process.
Task: Each process will perform histogram equalisation to its part of the image. Process 0 is master process. It is responsible for dividing the image and putting it together once equalisation have been done.
We can say that both data and task parallelisms were achieved<br /><br />
2.<b>Communication</b>:
Process 0 broadcasts the size of the image to the other processes using MPI_Bcast.Process 0 sends an image chunk each to the other processes using MPI_Send. Once received,Each process processes its image chunk which is sent back to main and pieced together with the other processes’ images to create one complete photo<br /><br />
3.<b>Agglomeration</b>:
the function par_histogram_equalizer combines all the tasks to be done by each process<br /><br />
4.<b>Mapping</b>:
Each process calls the function par_histogram_equalizer.<br /><br />

Below is a visualisation of the program:
![image](https://user-images.githubusercontent.com/45368931/167444910-ecb11db4-8b6b-4162-ac44-a722aba4c920.png) <br /><br />

Compilation: ```mpicc main.c -lm -o main``` <br />
Running : ```mpirun -n <number of processes> ./main <input image> <output image>``` <br />
To suppress any warnings from the MPI library , run the following command : ```echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope```
