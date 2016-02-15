package com.torchandroid.cifardemo.lua;

import android.content.Context;
import android.content.res.AssetManager;
import android.content.pm.ApplicationInfo;

public class LuaManager {

	private final String TAG = "LuaManager";
	private static LuaManager mInstance;

	private long mTorchState;
	static {
		System.loadLibrary("torchdemo");
	}

	private native int getTopResult(long stateLocation, int width, int height,
			byte[] bitmapRGBData);

	private native float testTorchData(long stateLocation);

        public native long initTorch(AssetManager manager, String libdir);

	public native void destroyTorch(long stateLocation);

	public static LuaManager getLuaManager(Context context) {
		if (mInstance == null)
			mInstance = new LuaManager(context);
		return mInstance;
	}

	public int getTopRecognitionResult(int width, int height,
			byte[] bitmapRGBData) {
		return getTopResult(mTorchState, width, height, bitmapRGBData);
	}

	private LuaManager(Context context) {
	    ApplicationInfo info = context.getApplicationInfo();
	    mTorchState = initTorch(context.getAssets(), info.nativeLibraryDir);
	}

	@Override
	protected void finalize() throws Throwable {
		destroyTorch(mTorchState);
		super.finalize();
	}

	public float testTorchData() {
		return testTorchData(mTorchState);
	}
}
