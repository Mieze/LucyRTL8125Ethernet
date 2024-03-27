//
//  Apple80211.h
//  itlwm
//
//  Created by qcwap on 2020/9/4.
//  Copyright © 2020 钟先耀. All rights reserved.
//

#ifndef Apple80211_h
#define Apple80211_h

#include <IOKit/80211/apple_private_spi.h>
#include <IOKit/80211/debug.h>
#include <IOKit/80211/IO80211WorkLoop.h>
#include <IOKit/80211/IO80211Controller.h>
#include <IOKit/80211/IO80211Interface.h>
#include <IOKit/80211/IO80211VirtualInterface.h>
#include <IOKit/80211/IO80211P2PInterface.h>
#if __IO80211_TARGET >= __MAC_10_15
#include <IOKit/80211/IO80211SkywalkInterface.h>
#endif

#endif /* Apple80211_h */
