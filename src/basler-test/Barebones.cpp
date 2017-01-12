#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <iostream>

//#define PARAM BalanceWhiteAuto
//#define SPARAM "BalanceWhiteAuto"
//#define VALUE Basler_GigECamera::BalanceWhiteAuto_Continuous

#define PARAM GainAuto
#define SPARAM "GainAuto"
#define VALUE Basler_GigECamera::GainAuto_Once

int main(int argc, char* argv[]) {
	Pylon::PylonInitialize();

	Pylon::CBaslerGigEInstantCamera* camera =
			new Pylon::CBaslerGigEInstantCamera(
					Pylon::CTlFactory::GetInstance().CreateFirstDevice());

	camera->Open();

	GenApi::EAccessMode access = camera->PARAM.GetAccessMode();

	if (!GenApi::IsWritable(access)) {
		std::cout << SPARAM << " (" << access << ") is not writable, exiting."
				<< std::endl;
		Pylon::PylonTerminate();
		return 1;
	} else {
		std::cout << SPARAM << " (" << access
				<< ") is writable, attempting set to " << VALUE<< std::endl;
	}

	try {
		camera->PARAM.SetValue(VALUE);
		if (camera->PARAM.GetValue(true, true) == VALUE) {
			std::cout << "Value set successfully." << std::endl;
		} else {
			std::cout << "No error, but not set correctly: " << camera->PARAM.GetValue(true, true) << std::endl;
		}
	} catch (Pylon::GenericException& e) {
		std::cerr << e.what() << std::endl;
	}
	camera->Close();

	delete camera;

	Pylon::PylonTerminate();
}
