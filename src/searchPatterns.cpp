#include "utils.hpp"
#include "utilsCV.hpp"

using namespace cv;
using namespace std;

// Detectors, descriptors, loadImage
const string detectorType = "SIFT";
const string descriptorType = "SIFT";
const int color = 0;
int numImagesTotal = 0;

// K-means
const int clusterCount = 282;  // K const in k-means. This must be <= Total number of rows in the sum of all vocabulary images.
const int attempts = 3;

// Directories, files
const string vocabularyImagesNameFile = "/Users/xescriche/git/SearchPatternsInArt/tests/test2/vocabularyImages.txt";
const string newImageFileName = "/Users/xescriche/git/SearchPatternsInArt/tests/test2/tapies9.jpg";
const string dirToSaveResImages = "/Users/xescriche/git/SearchPatternsInArt/tests/test2/results";

void computeMatching() {
	try {

	// POINT 1: DEFINE Feature detector (detectorType) AND Descriptor extractor (descriptorType)

		// Feature detector
		Ptr<FeatureDetector> featureDetector;
		featureDetector = FeatureDetector::create(detectorType);
		if (featureDetector.empty())
			cout << "The detector cannot be created." << endl << ">" << endl;

		// Descriptor extractor
		Ptr<DescriptorExtractor> descriptorExtractor;
		descriptorExtractor = DescriptorExtractor::create(descriptorType);
		if (featureDetector.empty())
			cout << "The descriptor cannot be created." << endl << ">" << endl;

	// POINT 2.1: READ IMAGES, DETECT KEYPOINTS AND EXTRACT DESCRIPTORS ON "VOCABULARY IMAGES"

		vector<Mat> vocabularyImages;
		vector<string> vocabularyImagesNames;
		if (!readImagesFromFile(vocabularyImagesNameFile, vocabularyImages,vocabularyImagesNames, color, numImagesTotal))
			cout << endl;

		vector<vector<KeyPoint> > vocabularyImagesKeypoints;
		detectKeypointsImagesVector(vocabularyImages, vocabularyImagesKeypoints, featureDetector);

		// Show the keypoints on screen
//		 showKeypointsImagesVector(vocabularyImages, vocabularyImagesKeypoints);

		vector<Mat> imagesVectorDescriptors;
		computeDescriptorsImagesVector(vocabularyImages, vocabularyImagesKeypoints,imagesVectorDescriptors, descriptorExtractor);

	// POINT 2.2: KMEANS ON imagesVectorDescriptors

		vector<vector<int> > vocabulary(clusterCount, vector<int>(numImagesTotal));
		Mat labels;
		Mat centers;
		kmeansVocabularyImages(imagesVectorDescriptors, clusterCount, attempts, numImagesTotal, vocabulary, labels, centers);

	// POINT 3.1: READ IMAGE, DETECT KEYPOINTS AND EXTRACT DESCRIPTORS ON "NEW IMAGE"

		Mat newImage;
		if (!readImage(newImageFileName,newImage,color))
			cout << endl;

		vector<KeyPoint> newImageKeypoints;
		detectKeypointsImage(newImage, newImageKeypoints, featureDetector);

		// Show the keypoints on screen
		// showKeypointsImage(newImage, newImageKeypoints);

		Mat newImageDescriptors;
		computeDescriptorsImage(newImage, newImageKeypoints, newImageDescriptors, descriptorExtractor);

	//  POINT 3.2: Find Words/KCenters on newImageDescriptors

		Mat wordsNewImage(newImageDescriptors.rows, 1, centers.type());
		findKCentersOnNewImage(wordsNewImage, newImageDescriptors, clusterCount, attempts, labels, centers);

	// POINT 3.3: Voting images

		int mostVotedImage = votingImages(vocabulary,wordsNewImage,numImagesTotal);

	// POINT 4.1: RANSAC

		Mat imageSelected = vocabularyImages[mostVotedImage];
		vector<KeyPoint> imageSelectedKeypoints = vocabularyImagesKeypoints[mostVotedImage];
		Mat imageSelectedDescriptors = imagesVectorDescriptors[mostVotedImage];
		Mat wordsImageIni(imageSelectedDescriptors.rows, 1, centers.type());

		findKCentersOnNewImage(wordsImageIni, imageSelectedDescriptors, clusterCount, attempts, labels, centers);

//		showMatrixValues3(imageSelectedKeypoints,wordsImageIni, "wordsImageIni:");
//		showMatrixValues3(newImageKeypoints, wordsNewImage, "wordsNewImage:");
//		showKeypointsImage(imageSelected, imageSelectedKeypoints);
//		showKeypointsImage(newImage, newImageKeypoints);

		ransac(wordsImageIni, wordsNewImage, imageSelected, imageSelectedKeypoints, newImage, newImageKeypoints);

   } catch (exception& e) {
		cout << e.what() << endl;
	}
}

int main(int argc, char *argv[]) {
	computeMatching();
}
