package com.torch;

import android.util.Log;
import android.content.res.AssetManager;
import android.content.pm.ApplicationInfo;
import android.content.Context;
import android.os.AsyncTask;

public class Torch
{
    AssetManager    assetManager;
    ApplicationInfo info;

    // native method
    private native String  jni_call(AssetManager manager, String path, String luafile);

    public Torch() {}

    public Torch setContext(Context myContext) {
	assetManager = myContext.getAssets();
        info = myContext.getApplicationInfo();
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("cublas");
        System.loadLibrary("THC");
        System.loadLibrary("cutorch");
        System.loadLibrary("THNN");
        System.loadLibrary("THCUNN");
        System.loadLibrary("torchandroid");
	Log.d("Torch","Torch() called\n");
	return this;
    }

    public class EvalAssetFileTask extends AsyncTask<String, Integer, String>
    {
	protected String doInBackground(String... lua) {
	    int count = lua.length;
	    String response = "";
	    for (int i = 0; i < count; i++) {
		Log.d("doInBackground(%s)\n", lua[i]);
		response += evalAssetFile(lua[i]);
	    }
	    return response;
	}	
	Torch mTorch;
    };
	
	// Todo: extract native method to evaluate Lua String
    private String evalAssetFile(String lua) {
	Log.d("Torch.evalAssetFile(%s)\n", lua);
	return jni_call(assetManager, info.nativeLibraryDir, lua);
    }

}
