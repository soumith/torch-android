package com.torchandroid.facedemo;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;
import android.content.pm.ApplicationInfo;

public class CameraActivity extends Activity
{

	private FrameLayout mainLayout;
	private ImageView MyCameraClass = null;
	private CameraClass camPreview;
	private int PreviewSizeWidth, PreviewSizeHeight;
	static AssetManager assetManager;
        static String nativeLibraryDir;
	static TextView detailsText;
	Button back;
	
	public void onCreate(Bundle savedInstanceState) 
	{
	    super.onCreate(savedInstanceState);
 	
	    getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,  
	              WindowManager.LayoutParams.FLAG_FULLSCREEN);
	    	    
	    requestWindowFeature(Window.FEATURE_NO_TITLE);  
	    
	    assetManager = getAssets();
	    ApplicationInfo info = getApplicationInfo();
	    nativeLibraryDir = info.nativeLibraryDir;

	    setContentView(R.layout.activity_camera);

	    back = (Button) findViewById(R.id.back);
	    back.setOnClickListener(buttonHandler);
	    
	    detailsText = (TextView) this.findViewById(R.id.details);
	    	    
	    MyCameraClass = new ImageView(this);
		 
	    SurfaceView camView = new SurfaceView(this);
	    SurfaceHolder camHolder = camView.getHolder();
	    
	    PreviewSizeWidth = MainActivity.width;
	    PreviewSizeHeight = MainActivity.height;
	    
	    camPreview = new CameraClass(PreviewSizeWidth, PreviewSizeHeight, MyCameraClass);
	    camHolder.addCallback(camPreview);
	         
	    camHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	         
	    mainLayout = (FrameLayout) findViewById(R.id.frameLayout);
	    mainLayout.addView(camView, new LayoutParams(1920, 1080));
	    mainLayout.addView(MyCameraClass, new LayoutParams(1920, 1080));
	    
	}
	
	View.OnClickListener buttonHandler = new View.OnClickListener() 
	{
		@Override
		public void onClick(View v) 
		{
			Intent backToMainActivity = new Intent(CameraActivity.this, MainActivity.class);
			CameraActivity.this.startActivity(backToMainActivity);
			finish();
			
		}
	};
}
