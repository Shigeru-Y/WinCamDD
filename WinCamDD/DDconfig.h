//
// Definition for Defect Detection.
//

#define MAX_PROC_IMAGES 10

CLASS int distMode;
CLASS int paramThresh;
CLASS int ddThresh;
CLASS int shrinkSteps;
CLASS BOOL fUseProfile;

// QR code reading.
CLASS BOOL fQRread;

// Log take
CLASS BOOL fLogtake;

#define MAX_DEFECTS 10
#define CLUSTER_SIZE 100
#define MIN_DEFECT_PIXELS 10
#define BIG_DEFECT   160

#define MAX_TRAPEZOID  50

typedef struct {
	BOOL fDefect;
	int dfMinX, dfMaxX, dfMinY, dfMaxY;
	int pixels;
	int positive;
	char attribute[100];
} DEFECT;

CLASS DEFECT defects[MAX_DEFECTS];
CLASS int defectBottom;
