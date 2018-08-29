#include "scanparameter.h"

uint8_t ScanParameter::Kv;
uint16_t ScanParameter::Ma;

FSS ScanParameter::FocalSpotSize;   // Small, Medium, Large
uint8_t ScanParameter::GeneratorMode;   // 0x00=Normal, 0x69=PreAdjust, 0x52=FinalAdjust
uint8_t ScanParameter::AnodeSpeed;      // 0 ~ 255Hz

uint32_t ScanParameter::ExposureTimeInMSec;
uint32_t ScanParameter::ScanTimeInMSec;

bool ScanParameter::ImaEnabled;
uint16_t ScanParameter::ImaStartingMa;
uint16_t ScanParameter::ImaMaxMa;

bool ScanParameter::XDitherEnabled;
bool ScanParameter::ZDitherEnabled;

uint32_t ScanParameter::IntegrationLimit;
unsigned ScanParameter::IntegrationTime;

unsigned ScanParameter::TriggerMode;
uint32_t ScanParameter::TriggerPosition;
