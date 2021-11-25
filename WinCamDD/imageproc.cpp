//
// Image processing
//
#include "pch.h"

#define CLASS extern
#include "usbCamera.h"
#include "DDconfig.h"

int histogram[256];
int globalMax, globalMin, globalThresh;

#define MAX_X_DIST 1280

int distHist[MAX_X_DIST];


BOOL IMP_GetHist( IMG0 *src )
{
	int x, y, index, gray;
	unsigned char *uc;
	
	for( gray = 0 ; gray < 256 ; gray++ ){
		histogram[gray] = 0;
	}

	uc = (unsigned char *)src->lpBmpData;
	for( y = 0 ; y < src->bih.biHeight ; y++ ){
		index = y * src->bih.biWidth * 3;
		for( x = 0 ; x < src->bih.biWidth ; x++, index += 3 ){
			gray = uc[index];
			histogram[gray]++;
		}
	}
	return TRUE;
}

int IMP_Threshold( void )
{
	int i, total, sum;

	// Get total pixels
	total = 0;
	for( i = 0 ; i < 256 ; i++ ){
		total += histogram[i];
	}

	// Get minimum Gray;
	sum = 0;
	for( i = 0 ; i < 256 ; i++ ){
		sum += histogram[i];
		if( sum >= total / 10 ){
			break;
		}
	}
	globalMin = i;

	// Get maximum Gray;
	sum = 0;
	for( i = 255 ; i >= 0 ; i-- ){
		sum += histogram[i];
		if( sum >= total / 10 ){
			break;
		}
	}
	globalMax = i;
	globalThresh = (globalMax + globalMin) / 2;

	// Determine DD threshold
	// ddThresh = (globalMax - globalMin) / 2;
	// ddThresh = (globalMax - globalMin) / 3;

	// 2021/Nov/21 S.Yamamoto  Parameterize.
	ddThresh = (globalMax - globalMin) * paramThresh / 100;

	return globalThresh;
}

// 
// Binarize
//
BOOL IMP_Binarize( IMG0 *src, IMG0 *dest )
{
	int x, y, index, gray, localMax, localMin, localThresh;
	unsigned char *ucs, *ucd;
	
	ucs = (unsigned char *)src->lpBmpData;
	ucd = (unsigned char *)dest->lpBmpData;
	for( y = 0 ; y < src->bih.biHeight ; y++ ){
		index = y * src->bih.biWidth * 3;
		localMax = globalMax;
		localMin = globalMin;
		localThresh = globalThresh;
		for( x = 0 ; x < src->bih.biWidth ; x++, index += 3 ){
			gray = ucs[index];
			if( gray > localThresh ){
				ucd[index] = 255;
				ucd[index+1] = 0;
				ucd[index+2] = 0;
//				// Update local
//				localMax = (localMax * 3 + gray) / 4;
			} else {
				ucd[index] = 0;
				ucd[index+1] = 64;
				ucd[index+2] = 64;
//				// Update local
//				localMin = (localMin * 3 + gray) / 4;
			}
//			localThresh = (localMax + localMin)/2;
		}
	}
	return TRUE;
}

//BOOL IMP_Binarize( IMG0 *src, IMG0 *dest )
//{
//	int x, y, index, gray, localMax, localMin, localThresh;
//	unsigned char *ucs, *ucd;
//	
//	ucs = (unsigned char *)src->lpBmpData;
//	ucd = (unsigned char *)dest->lpBmpData;
//	for( y = 0 ; y < src->bih.biHeight ; y++ ){
//		index = y * src->bih.biWidth * 3;
//		localMax = globalMax;
//		localMin = globalMin;
//		localThresh = globalThresh;
//		for( x = 0 ; x < src->bih.biWidth ; x++, index += 3 ){
//			gray = ucs[index];
//			if( gray > localThresh ){
//				ucd[index] = 255;
//				ucd[index+1] = 0;
//				ucd[index+2] = 0;
//				// Update local
//				localMax = (localMax * 3 + gray) / 4;
//			} else {
//				ucd[index] = 64;
//				ucd[index+1] = 64;
//				ucd[index+2] = 64;
//				// Update local
//				localMin = (localMin * 3 + gray) / 4;
//			}
//			localThresh = (localMax + localMin)/2;
//		}
//	}
//	return TRUE;
//}
//
//
// Shading compensation
//
BOOL	IMP_ShadingCompensation( IMG0 *src, IMG0 *shade, IMG0 *dest )
{
	int x, y, xx, yy, index, ii, gray, maxGray, totalMax;
	int compensation;
	unsigned char *ucS, *ucShade, *ucD;

	// Get shade
	ucS = (unsigned char *)src->lpBmpData;
	ucShade = (unsigned char *)shade->lpBmpData;
	ucD = (unsigned char *)dest->lpBmpData;
	totalMax = 0;

	for( y = 0 ; y < src->bih.biHeight ; y++ ){
		index = y * src->bih.biWidth * 3;
		for( x = 0 ; x < src->bih.biWidth ; x++, index += 3 ){
			// Search maximum gray in 20x20 area.
			maxGray = 0;
			for( yy = y - 10 ; yy < y + 10 ; yy += 2 ){
				if( yy < 0 || yy >= src->bih.biHeight ){
					continue;
				}
				ii = yy * src->bih.biWidth * 3;
				for( xx = x - 10 ; xx < x + 10 ; xx += 2 ){
					if( xx < 0 || xx >= src->bih.biWidth ){
						continue;
					}
					gray = ucS[ii + xx];
					if( gray > maxGray ){
						maxGray = gray;
					}
				}
			}
			ucShade[index + x] = maxGray;
			ucShade[index + x + 1] = maxGray;
			ucShade[index + x + 2] = maxGray;

			if( maxGray > totalMax ){
				totalMax = maxGray;
			}
		}
	}

	// Shading compensation;
	for( y = 0 ; y < src->bih.biHeight ; y++ ){
		index = y * src->bih.biWidth * 3;
		for( x = 0 ; x < src->bih.biWidth ; x++, index += 3 ){
			// Get compensation.
			compensation = totalMax - ucShade[index + x];
			ucD[index + x] = ucS[index + x] + compensation;
			ucD[index + x + 1] = ucS[index + x + 1] + compensation;
			ucD[index + x + 2] = ucS[index + x + 2] + compensation;
		}
	}
	return TRUE;
}

BOOL IMP_TrapezoidCompensation( IMG0 *src, IMG0 *dest )
{
	int x, y, xx, index, sIndex, dIndex;
//	int compensation;
	double xStretchRatio;
	unsigned char *ucS, *ucD;

	// Get compensation
	ucS = (unsigned char *)src->lpBmpData;
	ucD = (unsigned char *)dest->lpBmpData;

	for( y = 0 ; y < dest->bih.biHeight ; y++ ){
		dIndex = y * dest->bih.biWidth * 3;
		xStretchRatio = (double)(y * trapezoidCompensation * 10) / (double)(dest->bih.biHeight * dest->bih.biWidth);		// 0 - 1.0
		
		for( x = 0, index = dIndex ; x < dest->bih.biWidth ; x++, index += 3 ){
			// Calculate source index.
			xx = x - (int)((double)(x - dest->bih.biWidth / 2) * xStretchRatio);
//			xx = src->bih.biWidth / 2 + ( x - dest->bih.biWidth / 2) * xStretchRatio;
			if( xx < 0 || xx >= src->bih.biWidth ){
				// Out of source image.
				ucD[index] = 0;
				ucD[index+1] = 0;
				ucD[index+2] = 0;
//				continue;
			} else {
				sIndex = dIndex + xx * 3;
				ucD[index] = ucS[sIndex];
				ucD[index+1] = ucS[sIndex+1];
				ucD[index+2] = ucS[sIndex+2];
			}
		}
	}

	return TRUE;
}


//
// Image copy 
//
BOOL IMP_ImageCopy( IMG0 *src, IMG0 *dest )
{
	memcpy( dest->lpBmpData, src->lpBmpData, src->bih.biSizeImage );
	return TRUE;
}

// Get horizontal repeat with vertical profile.
int IMP_GetRepeatWithProfile( IMG0 *src )
{
	int verticalProfile[4096];	// Ensure enough number for horizontal resolution.
	unsigned char *ucs;
	int x, y, index;

	// Get vertical profile.
	memset(verticalProfile, 0, sizeof(verticalProfile));

	ucs = (unsigned char*)src->lpBmpData;
	for (y = 0; y < src->bih.biHeight; y++) {
		index = y * src->bih.biWidth * 3;
		for (x = 0; x < src->bih.biWidth; x++, index += 3) {
			// Get profile.
			verticalProfile[x] += (int)(ucs[index]);
		}
	}

	// Analyze profile.
	if (fLogtake) {
		FILE* fp;
		errno_t error;
		error = fopen_s(&fp, "c:\\temp\\logs\\WinCamDD_vertical_profile.csv", "w");
		if (error == 0) {
			for (x = 0; x < src->bih.biWidth; x++) {
				fprintf(fp, "%d,%d\n", x, verticalProfile[x]);
			}
			fclose(fp);
		}
	}

	// Determine threshold.
	int maxProfile = 0;
	int profileThresh;
	for (x = 0; x < src->bih.biWidth; x++) {
		if (maxProfile < verticalProfile[x]) {
			maxProfile = verticalProfile[x];
		}
	}
	profileThresh = maxProfile * 9 / 10;	// Set threshold at 90% of maximum.

	// Get average.
	int nEdges = -1, lastUpCross = 0;
	int sumEdgeDist = 0;

	for (x = 0; x < src->bih.biWidth - 1; x++) {
		if (verticalProfile[x] <= profileThresh && verticalProfile[x + 1] > profileThresh) {
			// Find edge.
			if (nEdges < 0) {
				// This is first edge.
				nEdges = 0;
				lastUpCross = x;
			}
			else {
				// After second.
				sumEdgeDist += (x - lastUpCross);
				nEdges += 1;
				lastUpCross = x;
			}
		}
	}
	if (nEdges > 0) {
		// Found relevant edge number.
		distMode = sumEdgeDist / nEdges;
	}
	else {
		distMode = 10;		// Avoid catastrophe.
	}

	return TRUE;
}


// Get horizontal repeat
int IMP_GetHorizontalRepeat( IMG0 *src )
{
	int x, y, index, dist, maxMode, lastEdge;
	unsigned char *ucs;
	
	// Clear distance histogram;
	for( x = 0 ; x < MAX_X_DIST ; x++ ){
		distHist[x] = 0;
	}

	ucs = (unsigned char *)src->lpBmpData;
	for( y = 0 ; y < src->bih.biHeight ; y++ ){
		index = y * src->bih.biWidth * 3;
		lastEdge = -1;	// Indicate no last edge.
		for( x = 0 ; x < src->bih.biWidth - 3 ; x++, index += 3 ){
			// Try to find edge.
			if( ucs[index] == 0 && ucs[index+3] != 0 ){
				// Find positive edge.
				if( lastEdge >= 0 ){
					// Last edge found.
					dist = x - lastEdge;
					distHist[dist]++;
				}

				// Save last edge.
				lastEdge = x;
			}
		}
	}

	// Leave data log.
	if (fLogtake) {
		FILE* fp;
		errno_t error;
		error = fopen_s(&fp, "c:\\temp\\logs\\WinCamDD_repeat_profile.csv", "w");
		if (error == 0) {
			for (x = 1; x < MAX_X_DIST; x++) {
				fprintf(fp, "%d,%d\n", x, distHist[x]);
			}
			fclose(fp);
		}
	}

	// Get maximum mode;
	distMode = 1;
	maxMode = 1;

	for( x = 1 ; x < MAX_X_DIST ; x++ ){
		if( distHist[x] > maxMode ){
			distMode = x;
			maxMode = distHist[x];
		}
	}

	return TRUE;
}


// Defect Detection.
// 2021/Nov/19 S.Yamamoto  Added neighbor test.
// 2021/Nov/19 S.Yamamoto  Added distortion thresholding.
//
int curImageWidth = 0;
int distThreshold[2048];	// Must be bigger than maximum image width.

int IMP_Subtract( IMG0 *src, IMG0 *dest )
{
	int x, y, index, rIndex, cIndex, fIndex;
	int tmpThresh, distance, distCoef;
	unsigned char *ucs, *ucd;
	
	ucs = (unsigned char *)src->lpBmpData;
	ucd = (unsigned char *)dest->lpBmpData;
	
	// Make dist threshold.
//	if (curImageWidth != src->bih.biWidth) {
		// distCoef = src->bih.biWidth * src->bih.biWidth / 4;
		distCoef = src->bih.biWidth * src->bih.biWidth / 10;
		for (x = 0; x < src->bih.biWidth; x++) {
			distance = x - src->bih.biWidth / 2;
			distThreshold[x] = ddThresh + (ddThresh * distance * distance / distCoef);
		}
		curImageWidth = src->bih.biWidth;
//	}

	// Clear destination.
	memset( ucd, 0, dest->bih.biWidth * dest->bih.biHeight * 3 );

	for( y = 0 ; y < src->bih.biHeight ; y++ ){
		index = y * src->bih.biWidth * 3;
		for( x = distMode * 2 ; x < src->bih.biWidth - distMode * 2 ; x++ ){
			// set index.
			rIndex = index + (x - distMode) * 3;
			cIndex = index + x * 3;
			fIndex = index + (x + distMode) * 3;
			// Set dd threshold.
			tmpThresh = distThreshold[x];

			// Exec DD
			if( (int)ucs[rIndex] - (int)ucs[cIndex] > tmpThresh &&
				(int)ucs[fIndex] - (int)ucs[cIndex] > tmpThresh){
				// Find candidate for dark.
				// Check neighbor.
				if (((int)ucs[rIndex] + (int)ucs[rIndex - 3]) / 2 - (int)ucs[cIndex] > tmpThresh &&
					((int)ucs[rIndex] + (int)ucs[rIndex + 3]) / 2 - (int)ucs[cIndex] > tmpThresh &&
					((int)ucs[fIndex] + (int)ucs[fIndex - 3]) / 2 - (int)ucs[cIndex] > tmpThresh &&
					((int)ucs[fIndex] + (int)ucs[fIndex + 3]) / 2 - (int)ucs[cIndex] > tmpThresh) {
					ucd[cIndex] = 128;
				}
			}
			if( (int)ucs[cIndex] - (int)ucs[rIndex] > tmpThresh &&
				(int)ucs[cIndex] - (int)ucs[fIndex] > tmpThresh){
				// Find candidate for bright.
				// Check neighbor.
				if ((int)ucs[cIndex] - ((int)ucs[rIndex] + (int)ucs[rIndex - 3]) / 2 > tmpThresh &&
					(int)ucs[cIndex] - ((int)ucs[rIndex] + (int)ucs[rIndex + 3]) / 2 > tmpThresh &&
					(int)ucs[cIndex] - ((int)ucs[fIndex] + (int)ucs[fIndex - 3]) / 2 > tmpThresh &&
					(int)ucs[cIndex] - ((int)ucs[fIndex] + (int)ucs[fIndex + 3]) / 2 > tmpThresh) {
					ucd[cIndex] = 129;
				}
			}
			//if( abs( ucs[rIndex] - ucs[cIndex] ) > ddThresh &&
			//	abs( ucs[fIndex] - ucs[cIndex] ) > ddThresh ){
			//	// Find candidate
			//	ucd[cIndex] = 128;
			//}
		}
	}

	return TRUE;
}

int IMP_Shrink( IMG0 *src, IMG0 *dest )
{
	int x, y, index, rIndex, cIndex, fIndex;
	unsigned char *ucs, *ucd;
	
	ucs = (unsigned char *)src->lpBmpData;
	ucd = (unsigned char *)dest->lpBmpData;
	
	// Clear destination.
	memset( ucd, 0, dest->bih.biWidth * dest->bih.biHeight * 3 );

	for( y = 1 ; y < src->bih.biHeight - 1 ; y++ ){
		index = y * src->bih.biWidth * 3;
		for( x = distMode ; x < src->bih.biWidth - distMode ; x++ ){
			// set index.
			cIndex = index + x * 3;
			if( ucs[cIndex] ){
				// Find defective.
				rIndex = cIndex - src->bih.biWidth * 3;
				fIndex = cIndex + src->bih.biWidth * 3;

				// Check neighbor
				if( ucs[cIndex - 3] && ucs[cIndex + 3] &&
					ucs[rIndex - 3] && ucs[rIndex] && ucs[rIndex + 3] &&
					ucs[fIndex - 3] && ucs[fIndex] && ucs[fIndex + 3] ){
					// All neighbors are defective.
					ucd[cIndex] = ucs[cIndex];
				}
			}
		}
	}

	return TRUE;
}

BOOL IMP_DefectDetect( IMG0 *src )
{
	int x, y, xx, yy, i, index, dfIndex;
//	int cTotal, positive;
	int miny, maxy;

	unsigned char *ucs;
//	unsigned char *ucd;

	// Reset defect number
	defectBottom = 0;

	for( i = 0 ; i < MAX_DEFECTS ; i++ ){
		// Preparation
		defects[i].dfMinX = src->bih.biWidth;
		defects[i].dfMaxX = 0;
		defects[i].dfMinY = src->bih.biHeight;
		defects[i].dfMaxY = 0;
		defects[i].pixels = 0;
		defects[i].positive = 0;
		defects[i].fDefect = FALSE;
		defects[i].attribute[0] = 0;	// Clear attribute text.

		ucs = (unsigned char *)src->lpBmpData;
		
		for( y = 1 ; y < src->bih.biHeight - 1 ; y++ ){
			index = y * src->bih.biWidth * 3;
			for( x = distMode ; x < src->bih.biWidth - distMode ; x++ ){
				// set index.
				if( ucs[index + x * 3] ){
					// Find defective.
					// Check cluster.
					for( yy = y - CLUSTER_SIZE; yy < y + CLUSTER_SIZE ; yy++ ){
						if( yy < 0 || yy >= src->bih.biHeight ){
							continue;
						}
						for( xx = x - CLUSTER_SIZE ; xx < x + CLUSTER_SIZE ; xx++ ){
							if( xx < 0 || xx >= src->bih.biWidth ){
								continue;
							}
							dfIndex = (yy * src->bih.biWidth + xx) * 3;
							if( ucs[dfIndex] ){
								// Find cluster defective.
								if( xx < defects[i].dfMinX ) defects[i].dfMinX = xx;
								if( xx > defects[i].dfMaxX ) defects[i].dfMaxX = xx;
								if( yy < defects[i].dfMinY ) defects[i].dfMinY = yy;
								if( yy > defects[i].dfMaxY ) defects[i].dfMaxY = yy;
								defects[i].pixels++;
								if( ucs[dfIndex] & 0x01 ){
									defects[i].positive++;
								}
								ucs[dfIndex] = 0;
							}
						}
					}
					if( defects[i].pixels >= MIN_DEFECT_PIXELS ){
						defects[i].fDefect = TRUE;
					}
					break;
				}
			}
			if( defects[i].fDefect ){
				break;
			}
		}

		// Check defect
		if( defects[i].dfMinX <= defects[i].dfMaxX ){
			// Find at least one pixel remaining as defective.
			defects[i].fDefect = TRUE;
			// Reverse location for vertical direction.
			maxy = src->bih.biHeight - defects[i].dfMinY - 1;
			miny = src->bih.biHeight - defects[i].dfMaxY - 1;
			defects[i].dfMinY = miny;
			defects[i].dfMaxY = maxy;

		} else {
			defects[i].fDefect = FALSE;
			defectBottom = i;
			break;		// End of defect search.
		}
	}

	return TRUE;
}


BOOL ImageProc(void)
{
	int threshold;

	// Shading compensation
//	IMP_ShadingCompensation( &img00, &proc[0], &proc[1] );

	// Trapezoid compensation
	if (trapezoidCompensation) {
		IMP_TrapezoidCompensation(&img00, &proc[0]);
	}
	else {
		IMP_ImageCopy(&img00, &proc[0]);
	}

	// Binarize
	// Get histogram
	IMP_GetHist(&proc[0]);
	threshold = IMP_Threshold();
	IMP_Binarize(&proc[0], &proc[1]);

	// Get horizontal distanse
	if (fUseProfile) {
		IMP_GetRepeatWithProfile(&proc[0]);
	}
	else{
		IMP_GetHorizontalRepeat(&proc[1]);
	}

	// DD subtraction
	IMP_Subtract( &proc[0], &proc[2] );

	// DD filter
	switch (shrinkSteps) {
	case 1:
		IMP_Shrink(&proc[2], &proc[4]);
		break;
	case 2:
		IMP_Shrink(&proc[2], &proc[3]);
		IMP_Shrink(&proc[3], &proc[4]);
		break;
	case 3:
		IMP_Shrink(&proc[2], &proc[4]);
		IMP_Shrink(&proc[4], &proc[3]);
		IMP_Shrink(&proc[3], &proc[4]);
		break;
	}
	IMP_DefectDetect( &proc[4] );

	// QR code reading.
	if (fQRread) {
		// Select biggest defect.
		int defectIndex = 0;
		int maxSize = 0, defectSize;
		if (defectBottom > 0) {
			for( int i = 0 ; i < defectBottom ;i++ ){
				defectSize = (defects[i].dfMaxX - defects[i].dfMinX) * (defects[i].dfMaxY - defects[i].dfMinY);
				if (maxSize < defectSize) {
					maxSize = defectSize;
					defectIndex = i;
				}
			}
		}

		// Set margin.
#define DF_MARGIN 10
		int minX = defects[defectIndex].dfMinX - DF_MARGIN;
		int maxX = defects[defectIndex].dfMaxX + DF_MARGIN; 
		int minY = defects[defectIndex].dfMinY - DF_MARGIN; 
		int maxY = defects[defectIndex].dfMaxY + DF_MARGIN;
		if (minX < 0) minX = 0;
		if (minY < 0) minY = 0;
		if (maxX >= proc[0].bih.biWidth) maxX = proc[0].bih.biWidth - 1;
		if (maxY >= proc[0].bih.biHeight) maxY = proc[0].bih.biHeight - 1;

		// Make new image
		IMG0 qrImg;
		qrImg.bih.biWidth = maxX - minX;
		qrImg.bih.biHeight = maxY - minY;
		qrImg.lpBmpData = new unsigned char[qrImg.bih.biWidth * qrImg.bih.biHeight];

		// Upside down.
		int index;
		unsigned char* ucs, *ucd;
		for (int j = 0; j < qrImg.bih.biHeight; j++) {
			index = (proc[0].bih.biHeight - (j + minY) - 1) * proc[0].bih.biWidth;
			ucs = proc[0].lpBmpData + (index + minX) * 3;
			ucd = qrImg.lpBmpData + j * qrImg.bih.biWidth;
			for (int i = 0; i < qrImg.bih.biWidth; i++) {
				ucd[i] = ucs[i*3];
			}
		}
		// Call QR read.
		IMP_readQR(&qrImg, defects[defectIndex].attribute);

		// Clean up.
		delete qrImg.lpBmpData;

		// Reset flag.
		// fQRread = FALSE;
	}


	fLogtake = FALSE;	// Clear fLogtake.

	return TRUE;
}
