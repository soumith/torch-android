package com.torchandroid.cifardemo.util;

import java.nio.ByteBuffer;

import com.torchandroid.cifardemo.R;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.util.Log;

public class Util {
	private static String TAG = "Util";
	public final static String[] classes = { "airplane", "automobile", "bird",
			"cat", "deer", "dog", "frog", "horse", "ship", "truck" };

	public final static int[][] imageDrawableIds = {
			{ R.drawable.airplane1, R.drawable.airplane2, R.drawable.airplane3,
					R.drawable.airplane4, R.drawable.airplane5,
					R.drawable.airplane6, R.drawable.airplane7,
					R.drawable.airplane8, R.drawable.airplane9,
					R.drawable.airplane10 },
			{ R.drawable.automobile1, R.drawable.automobile2,
					R.drawable.automobile3, R.drawable.automobile4,
					R.drawable.automobile5, R.drawable.automobile6,
					R.drawable.automobile7, R.drawable.automobile8,
					R.drawable.automobile9, R.drawable.automobile10 },
			{ R.drawable.bird1, R.drawable.bird2, R.drawable.bird3,
					R.drawable.bird4, R.drawable.bird5, R.drawable.bird6,
					R.drawable.bird7, R.drawable.bird8, R.drawable.bird9,
					R.drawable.bird10 },
			{ R.drawable.cat1, R.drawable.cat2, R.drawable.cat3,
					R.drawable.cat4, R.drawable.cat5, R.drawable.cat6,
					R.drawable.cat7, R.drawable.cat8, R.drawable.cat9,
					R.drawable.cat10 },
			{ R.drawable.deer1, R.drawable.deer2, R.drawable.deer3,
					R.drawable.deer4, R.drawable.deer5, R.drawable.deer6,
					R.drawable.deer7, R.drawable.deer8, R.drawable.deer9,
					R.drawable.deer10 },
			{ R.drawable.dog1, R.drawable.dog2, R.drawable.dog3,
					R.drawable.dog4, R.drawable.dog5, R.drawable.dog6,
					R.drawable.dog7, R.drawable.dog8, R.drawable.dog9,
					R.drawable.dog10 },
			{ R.drawable.frog1, R.drawable.frog2, R.drawable.frog3,
					R.drawable.frog4, R.drawable.frog5, R.drawable.frog6,
					R.drawable.frog7, R.drawable.frog8, R.drawable.frog9,
					R.drawable.frog10 },
			{ R.drawable.horse1, R.drawable.horse2, R.drawable.horse3,
					R.drawable.horse4, R.drawable.horse5, R.drawable.horse6,
					R.drawable.horse7, R.drawable.horse8, R.drawable.horse9,
					R.drawable.horse10 },
			{ R.drawable.ship1, R.drawable.ship2, R.drawable.ship3,
					R.drawable.ship4, R.drawable.ship5, R.drawable.ship6,
					R.drawable.ship7, R.drawable.ship8, R.drawable.ship9,
					R.drawable.ship10 },
			{ R.drawable.truck1, R.drawable.truck2, R.drawable.truck3,
					R.drawable.truck4, R.drawable.truck5, R.drawable.truck6,
					R.drawable.truck7, R.drawable.truck8, R.drawable.truck9,
					R.drawable.truck10 } };

	/**
	 * 
	 * @return Resource's RGBA byte array
	 */
	public static byte[] getImageRGBA(Resources res, int resouceId) {
		Bitmap bitmap = BitmapFactory.decodeResource(res, resouceId);
		if (bitmap.getWidth() != 32 || bitmap.getHeight() != 32) {
		}
		return getImageRGBA(bitmap);
	}

	/**
	 * 
	 * @return Bitmap's RGBA byte array
	 */
	public static byte[] getImageRGBA(Bitmap inputBitmap) {
		Config config = inputBitmap.getConfig();
		ByteBuffer buffer;

		Bitmap bitmap;
		/**
		 * if bitmap size is not 32*32 create scaled bitmap
		 */

		if (inputBitmap.getWidth() != 32 || inputBitmap.getHeight() != 32) {
			Log.d(TAG, "bitmap resized to 32x32");
			bitmap = Bitmap.createScaledBitmap(inputBitmap, 32, 32, false);
		} else {
			bitmap = inputBitmap;
		}
		/**
		 * if bitmap is not ARGB_8888 format, copy bitmap with ARGB_8888 format
		 */
		if (!config.equals(Bitmap.Config.ARGB_8888)) {
			Bitmap bitmapARBG = bitmap.copy(Bitmap.Config.ARGB_8888, false);
			buffer = ByteBuffer.allocate(bitmapARBG.getByteCount());
			bitmapARBG.copyPixelsToBuffer(buffer);
			bitmapARBG.recycle();
		} else {
			buffer = ByteBuffer.allocate(bitmap.getByteCount());
			bitmap.copyPixelsToBuffer(buffer);
		}
		return buffer.array();
	}
}
