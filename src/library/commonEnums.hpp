#ifndef  COMMONENUMSHPP
#define COMMONENUMSHPP

enum carrierOptionsType
{
NO_PHASE_INFO = 0,
L1_PHASE_INFO = 1,
L1_AND_L2_PHASE_INFO = 2
};

enum solutionOptionsType
{
SINGLE_BASESTATION = 0,
NETWORK = 1
};

enum authenticationMethodType
{
NONE = 'N',
BASIC = 'B',
DIGEST = 'D'
};

enum feeType
{
FREE = 'N',
FEE_REQUIRED = 'Y'
};


#endif
