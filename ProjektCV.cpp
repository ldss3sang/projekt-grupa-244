#include "stdafx.h"
#include <sstream>
#include "ColorSpace.h"
#include "GuiCommunication.h"
#include "AdaBoost.h"
#include "Cascade.h"
#include "ViolaJones.h"
#include <fstream>
#include "Image.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "ImageLoader.h"
#include "testCode.h" //kod koji sluzi za testiranje implementiranih razreda

using namespace std;

/**
* Sadrzi main metodu za pokretanje algoritma za ucenje, te testiranje kaskade.
*/

void testirajKaskadu( Cascade &kaskada ) {
	DirectoryLoader loaderTest("c:\\Images\\test", kaskada.colorSpace(), true, 1, false );
	vector< Image * > testSlike = loaderTest.loadNextImages();

	vector < Cascade > kaskade;
	kaskade.push_back( kaskada );

	while( testSlike.size() ) {
		testSlike[0]->evaluateCascade( kaskade, 1., 1.25, 4., 1, 0 );
		Image::writeTestData();
		Image::clearTestData();
		delete testSlike[0]; //oslobadanje memorije zauzete slikama za testiranje
		testSlike = loaderTest.loadNextImages();
	}
}

void testViolaJones(int colorspace) {
	DirectoryLoader loaderTrue("c:\\Images\\true", colorspace);
	DirectoryLoader loaderFalse("c:\\Dokumenti\\Nastava - FER\\Projekt\\ProjektCV - ViolaJones\\Images\\false", colorspace, true, 1000, true); //ucitava ih rekurzivno tako da ih je moguce stavit u vise direktorija
	DirectoryLoader loaderFalse2("c:\\Dokumenti\\Nastava - FER\\Projekt\\ProjektCV - ViolaJones\\Images\\false2", colorspace, true, 1000, true); //ucitava ih rekurzivno tako da ih je moguce stavit u vise direktorija
	MultipleDirectoryLoader loaderFalseMultiple( loaderFalse, loaderFalse2 ); // ucitavanje slika iz vise direktorija

	Feature::loadBaseFeatures("basefeatures.txt");

	int tmpChans[] = {0, 1, 2}; //kanali slike za koje se generiraju featuri (za LAB mozda bi bilo dobro izbacit L kanal)
	vector< int > channels(tmpChans, tmpChans + 3 );

	if (colorspace == ColorSpace::GRAY) { //ukoliko je GRAY onda samo nulti kanal uzmi
		channels.clear(); channels.push_back( 0 );
	}

	double falsePositivePerLayer = 0.005, truePositivePerLayer = 0.999;
	double falsePositive = 0.00001;

	//ispis podataka na ekran
	Feature::generateAll(24, 24, 2, 1.3f, channels);
	cout << "VIOLA JONES..." << endl << "ColorSpace = " + ColorSpace::getName(colorspace) << endl;
	cout << "Broj featura: " << Feature::generatedFeatures.size() << ", Channels: ";
	for(int i=0; i<channels.size(); i++) cout << channels[i] << ",";
	cout << endl;
	cout << "False positive per layer: " << falsePositivePerLayer << ", False positive: " << falsePositive << endl;
	cout << "True positive per layer: " << truePositivePerLayer << endl;

	Cascade kaskada(colorspace);

	ViolaJones kuso( loaderTrue, loaderFalseMultiple, 1000 );
	//kuso.buildCascade( 0.1, 0.999, 0.000001, kaskada ); //tesko za izgradit ovakvu kaskadu
	kuso.buildCascade( falsePositivePerLayer, truePositivePerLayer, falsePositive, kaskada ); 

	kaskada.saveCascade("KaskadaTest.cascade" );
	testirajKaskadu( kaskada );
}

void testViolaJonesLoadFromFile(string file) {
	Cascade kaskada( ColorSpace::RGB ); // default je RGB ali prilikom ucitavanja to se pravilno ucita
	kaskada.loadCascade(file);
	testirajKaskadu( kaskada );
}

void testCommunicationWithGui() {
	Image *slika;
	GuiCommunication::start();
	while( (slika = GuiCommunication::getNextImage()) != NULL ) {
		vector <Image::Rectangle > rj = slika->evaluateCascade( GuiCommunication::getCascade(), GuiCommunication::getPocetniScale(), 
																GuiCommunication::getStepScale(), GuiCommunication::getZavrsniScale(),
																GuiCommunication::getIsSlijedno(), GuiCommunication::getTreshold() );		
		GuiCommunication::saveResults( rj );
		delete slika;
	}
	GuiCommunication::sendResults();
}


void test(void) {
	//testCode::testColorSpace();
	//testCode::testLoader(ColorSpace::GRAY);
	//testCode::testMultipleLoader();
	//testCode::testNegativeLoader();
	//for(int i=0; i<10; i++) {
		//testCode::testBigVector();
		//testCode::testBigVector2();
	//}
	//exit(0);
}


void redirectOutput() {
	std::streambuf* cout_sbuf = std::cout.rdbuf(); // save original sbuf
	ofstream fout("podaci.log");	
    std::cout.rdbuf(fout.rdbuf()); // redirect 'cout' to a 'fout'  
}

int _tmain(int argc, _TCHAR* argv[])
{	
	//redirectOutput();

	//testViolaJones(ColorSpace::LAB);
	//testViolaJonesLoadFromFile("KaskadaTest.cascade");
	testViolaJonesLoadFromFile("c:\\Dokumenti\\Nastava - FER\\Projekt\\ProjektCV - ViolaJones\\Podaci - colorspace\\LAB - dobra.cascade");
	//testCommunicationWithGui();

	return 0;
}





//TODO: ne postoji validation set za testiranje nego
//		se to radi pomocu slika s kojima je izvodeno ucenje(tog levela kaskade)
//		zato je nekad problem sa ucenjem visih levela kaskade.

//		ispravit bug sa bigVector-om, nakon sto je gotov sa generiranjem svih feature-a onda se srusi
//		moguce da je problem u destruktoru