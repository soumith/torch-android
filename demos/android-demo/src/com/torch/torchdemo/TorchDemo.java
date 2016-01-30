package com.torch.torchdemo;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.util.Log;
import android.content.res.AssetManager;

public class TorchDemo extends Activity
{
    static AssetManager assetManager;
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
	Log.d("torchdemo","Hello from JAVA\n");
	assetManager = getAssets();
	TextView tv = new TextView(this);
	String returnFromC = callTorch(assetManager);
        tv.setText(returnFromC);
        setContentView(tv);
    }
    
    // native method
    public native String  callTorch(AssetManager manager);
    
    // load the shared library
    static {
        System.loadLibrary("torchdemo");
    }
}
