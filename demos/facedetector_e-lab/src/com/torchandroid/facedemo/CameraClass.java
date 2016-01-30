package com.torchandroid.facedemo;

import java.io.IOException;
import java.util.List;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Parameters;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.view.SurfaceHolder;
import android.widget.ImageView;

public class CameraClass implements SurfaceHolder.Callback, Camera.PreviewCallback
{
    private Camera mCamera = null;
    private ImageView MycameraClass = null;
    private Bitmap bitmap = null;
    private int[] pixels = null;
    private byte[] FrameData = null;
    private int imageFormat;
    private int PreviewSizeWidth;
    private int PreviewSizeHeight;
    private boolean bProcessing = false;
    float start, stop, networkLoop, wholeLoop, fps, displayLoop, networkPercentage = 0;
    Handler mHandler = new Handler(Looper.getMainLooper());
    public long torchState = 0;
    private boolean destroyFlag = false;

   
    public CameraClass(int PreviewlayoutWidth, int PreviewlayoutHeight, ImageView cameraClass)
    {
	PreviewSizeWidth = PreviewlayoutWidth;
	PreviewSizeHeight = PreviewlayoutHeight;
	MycameraClass = cameraClass;
	bitmap = Bitmap.createBitmap(1920, 1080, Bitmap.Config.ARGB_8888);
	pixels = new int[1920*1080];
    }
	  
    public void onPreviewFrame(byte[] arg0, Camera arg1) 
    {
	    
	// At preview mode, the frame data will push to here.
	if (imageFormat == ImageFormat.YV12)
	    {

		if ( !bProcessing )
		    {
			FrameData = arg0;    
			mHandler.post(runNetwork);
		    }
	    }
    }
	 
    @Override
	public void surfaceChanged(SurfaceHolder arg0, int arg1, int arg2, int arg3) 
    {
	Parameters parameters;
	   
	parameters = mCamera.getParameters();
    List<Camera.Size> sizes = parameters.getSupportedPreviewSizes();
	// Set the camera preview size
	parameters.setPreviewFormat(ImageFormat.YV12);
	parameters.setPreviewSize(sizes.get(0).width, sizes.get(0).height);
	   
	imageFormat = parameters.getPreviewFormat();
	   
	mCamera.setParameters(parameters);
	    
	mCamera.startPreview();    
    }
	 
    @Override
	public void surfaceCreated(SurfaceHolder arg0) 
    {
	init();
		
	if(Camera.getNumberOfCameras() >= 2)
	    mCamera = Camera.open(CameraInfo.CAMERA_FACING_FRONT);
	      
	else
	    mCamera = Camera.open(CameraInfo.CAMERA_FACING_BACK);
	try
	    {
	    	// If did not set the SurfaceHolder, the preview area will be black.
	    	mCamera.setPreviewDisplay(arg0);
	    	mCamera.setPreviewCallback(this);
	    } 
	catch (IOException e)
	    {
	    	mCamera.release();
	    	mCamera = null;
	    }
    }
	 
    @Override
	public void surfaceDestroyed(SurfaceHolder arg0) 
    {
	mCamera.setPreviewCallback(null);
	mCamera.stopPreview();
	mCamera.release();
	mCamera = null;
	destroy();
    }
 
    public native float callTorch(long stateLocation, int width, int height,
				  byte[] NV21FrameData, int[] pixels);
	 
    public native long initTorch(AssetManager manager);

    public native void destroyTorch(long stateLocation);
	 
    static 
    {
	System.loadLibrary("torchdemo");
    }
	 
    private Runnable runNetwork = new Runnable() 
	{
	    public void run() 
	    {
	    	if(!destroyFlag)
		    {
			bProcessing = true;		    
				
			start = SystemClock.elapsedRealtime();
			networkLoop = callTorch(torchState, PreviewSizeWidth, PreviewSizeHeight, FrameData, pixels); 
			stop = SystemClock.elapsedRealtime();
			wholeLoop = (float) ((stop-start)/1000);
			fps = 1/wholeLoop;
				
			bitmap.setPixels(pixels, 0, 1920, 0, 0, 1920, 1080);
			MycameraClass.setImageBitmap(bitmap);
	
			networkPercentage = (networkLoop/wholeLoop)*100;
			    
			CameraActivity.detailsText.setText("****Profiling information****\n\n" + "Time for network = " + networkLoop + " s\nTime for whole loop = " +
							   wholeLoop + " s\nNetwork takes " + networkPercentage + "% of time" + "\nFrame rate =" + fps);
			  
			bProcessing = false;
		    }
	    }
	};

    public void init()
    {
	if(torchState == 0)
	    torchState = initTorch(CameraActivity.assetManager);
    }
	
    public void destroy()
    {
	destroyFlag = true;
	while(bProcessing){}
	destroyTorch(torchState);
	torchState = 0;
    }
	
}
