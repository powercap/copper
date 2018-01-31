#ifndef _COPPER_CONSTANTS_H_
#define _COPPER_CONSTANTS_H_

#ifdef __cplusplus
extern "C" {
#endif

// filter constants
static const double X_HAT_MINUS_START  =   0.0;
static const double X_HAT_START        =   0.2;
static const double P_START            =   1.0;
static const double P_MINUS_START      =   0.0;
static const double H_START            =   0.0;
static const double K_START            =   0.0;
static const double Q_DEFAULT          =   0.00001;
static const double R_DEFAULT          =   0.01;

// xup constants
static const double P1_DEFAULT         =   0.0;
static const double P2_DEFAULT         =   0.0;
static const double Z1_DEFAULT         =   0.0;

static const double MU_DEFAULT         =   1.0;
static const double E_START            =   0.0;
static const double EO_START           =   0.0;
static const double GAIN_LIMIT_DEFAULT =   0.0;
static const double EPC_DEFAULT        =   0.05;

#ifdef __cplusplus
}
#endif

#endif
