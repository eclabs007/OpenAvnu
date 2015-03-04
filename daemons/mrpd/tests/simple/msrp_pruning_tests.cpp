/******************************************************************************

  Copyright (c) 2014, AudioScience, Inc.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   3. Neither the name of the AudioScience, Inc nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef __linux__
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#else
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#define PRIu64       "I64u"
#define PRIx64       "I64x"
#endif

#include "CppUTest/TestHarness.h"

extern "C"
{

#include "mrp_doubles.h"
#include "msrp_tests.h"
#include "mrp.h"
#include "msrp.h"
#include "parse.h"
#include "eui64set.h"

/* Most MSRP commands operate on the global DB */
extern struct msrp_database *MSRP_db;

void msrp_event_observer(int event, struct msrp_attribute *attr);
char *msrp_attrib_type_string(int t);
char *mrp_event_string(int e);

}

/* This is from a live capture; it contains several messages with Mt
* and JoinMt events for Talker Advertise, Listener, and Domain
* VectorAttributes. */
static unsigned char pkt2[] = {
	0x01, 0x80, 0xc2, 0x00, 0x00, 0x0e, /* Destination MAC */
	0x00, 0x0f, 0xd7, 0x00, 0x23, 0x58, /* Source MAC */
	0x22, 0xea,                         /* Ethertype */

	0x00,         /* Protocol Version */

	/* Message Start */
	0x01,         /* Attribute Type - Talker Advertise */
	0x19,         /* Attribute FirstValue Length */
	0x00, 0xc4,   /* Attribute ListLength */

	/* Vector Header */
	0x00, 0x01,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x00, 0x23, 0x4d, 0x00, 0x00, /* ID 0x000fd700234d0000 */
	0x91, 0xe0, 0xf0, 0x00, 0xb7, 0x1a, 0x00, 0x00,
	0x00, 0x38, 0x00, 0x01, 0x60, 0x00, 0x02, 0x1f,
	0xd8,
	/* ThreePackedEvents */
	0x90,

	/* Vector Header */
	0x00, 0x13,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x00, 0x23, 0x4d, 0x00, 0x03,
	0x91, 0xe0, 0xf0, 0x00, 0xb7, 0x1d, 0x00, 0x00,
	0x00, 0x38, 0x00, 0x01, 0x70, 0x00, 0x02, 0x1f,
	0xd8,
	/* ThreePackedEvents */
	0xac, 0xac, 0xac, 0xac, 0xac, 0xac, 0x90,

	/* Vector Header */
	0x00, 0x0d,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x00, 0x23, 0x58, 0x00, 0x01,
	0x91, 0xe0, 0xf0, 0x00, 0x88, 0x3d, 0x00, 0x00,
	0x00, 0x38, 0x00, 0x01, 0x70, 0x00, 0x00, 0x01,
	0xf4,
	/* ThreePackedEvents */
	0x81, 0x81, 0x81, 0x81, 0x6c,

	/* Vector Header */
	0x00, 0x13,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x00, 0x23, 0xba, 0x00, 0x03,
	0x91, 0xe0, 0xf0, 0x00, 0x4c, 0x40, 0x00, 0x00,
	0x00, 0x38, 0x00, 0x01, 0x70, 0x00, 0x02, 0x1f,
	0xd8,
	/* ThreePackedEvents */
	0xac, 0xac, 0xac, 0xac, 0xac, 0xac, 0x90,

	/* Vector Header */
	0x00, 0x06,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x01, 0xa0, 0x05, 0x00, 0x01,
	0x91, 0xe0, 0xf0, 0x00, 0xdd, 0xd1, 0x00, 0x00,
	0x00, 0x38, 0x00, 0x01, 0x73, 0x00, 0x02, 0x1f,
	0xd8,
	/* ThreePackedEvents */
	0xac, 0xac,

	/* Vector Header */
	0x00, 0x1c,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x01, 0xa0, 0x16, 0x00, 0x03,
	0x91, 0xe0, 0xf0, 0x00, 0x6f, 0xdc, 0x00, 0x00,
	0x00, 0x38, 0x00, 0x01, 0x70, 0x00, 0x02, 0x1f,
	0xd8,
	/* ThreePackedEvents */
	0xac, 0xac, 0xac, 0xac, 0xac, 0xac, 0xac, 0xac,
	0xac, 0x90,

	0x00, 0x00, /* EndMark */

	/* Message Start */
	0x03,         /* Attribute Type - Listener */
	0x08,         /* Attribute FirstValue Length */
	0x00, 0x63,   /* Attribute ListLength */

	/* Vector Header */
	0x00, 0x01,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x00, 0x23, 0x4d, 0x01, 0x00,
	/* ThreePackedEvents */
	0x6c,
	/* FourPackedEvents */
	0x80,

	/* Vector Header */
	0x00, 0x13,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x00, 0x23, 0x4d, 0x02, 0x03,
	/* ThreePackedEvents */
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x6c,
	/* FourPackedEvents */
	0xaa, 0xaa, 0xaa, 0xaa, 0xa8,

	/* Vector Header */
	0x00, 0x13,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x00, 0x23, 0xba, 0x003, 0x03,
	/* ThreePackedEvents */
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x6c,
	/* FourPackedEvents */
	0xaa, 0xaa, 0xaa, 0xaa, 0xa8,

	/* Vector Header */
	0x00, 0x06,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x01, 0xa0, 0x05, 0x04, 0x01,
	/* ThreePackedEvents */
	0x81, 0x81,
	/* FourPackedEvents */
	0xaa, 0xa0,

	/* Vector Header */
	0x00, 0x1c,
	/* FirstValue */
	0x00, 0x0f, 0xd7, 0x01, 0xa0, 0x16, 0x05, 0x03,
	/* ThreePackedEvents */
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x6c,
	/* FourPackedEvents */
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,

	0x00, 0x00, /* EndMark */

	/* Message Start */
	0x04,          /* Attribute Type - Domain */
	0x04,          /* Attribute FirstValue Length */
	0x00, 0x09,    /* Attribute ListLength */

	/* Vector Header */
	0x00, 0x01,
	/* FirstValue */
	0x06, 0x03, 0x00, 0x02,
	/* ThreePackedEvents */
	0x6c,

	0x00, 0x00, /* EndMark */

	0x00, 0x00  /* EndMark */
};



// Various parameters used by MMRP, MVRP and MSRP
// (Note: Defined here in an effort to make it easier to
//        understand examples below when it comes time
//        to build commands in real code!)
#define STREAM_DA                "010203040506"
#define STREAM_ID                "DEADBEEFBADFCA11"
#define VLAN_ID                  "0002"
#define TSPEC_MAX_FRAME_SIZE     "576"
#define TSPEC_MAX_FRAME_INTERVAL "8000"
#define PRIORITY_AND_RANK        "96"
#define ACCUMULATED_LATENCY      "1000"
#define SR_CLASS_ID              "6"
#define SR_CLASS_PRIORITY        "3"
#define BRIDGE_ID                "BADC0FFEEC0FFEE0"
#define FAILURE_CODE             "1"

#define ST_PLUS_PLUS "S++:S=" STREAM_ID \
                     ",A=" STREAM_DA \
                     ",V=" VLAN_ID \
                     ",Z=" TSPEC_MAX_FRAME_SIZE \
                     ",I=" TSPEC_MAX_FRAME_INTERVAL \
                     ",P=" PRIORITY_AND_RANK \
                     ",L=" ACCUMULATED_LATENCY

static struct sockaddr_in client;

TEST_GROUP(MsrpPruningTestGroup)
{
	void setup()
	{
		mrpd_reset();
		msrp_init(1, MSRP_INTERESTING_STREAM_ID_COUNT, 1);
	}

	void teardown()
	{
		msrp_reset();
		mrpd_reset();
	}
};

/*
 * When interesting StreamID tracking is enabled only a single
 * client is supported correctly as there are no notifications
 * sent to client B when client A removes an ID and client B is
 * also "attached". Multiple clients are therefore not allowed
 * when stream ID pruning is enabled and the second client
 * will have an error returned.
 *
 * Note: mrpd should be started without the command line parameter
 * to enable interesting stream tracking if multiple clients are
 * going to be using mrpd. A notification mechanism between clients
 * would need to be added if multiple client support was to be correctly
 * implemented. And most likely either a reference count and/or a bitmask
 * added to the database where the interesting stream IDs are stored.
 * An API for dumping/reporting current interesting streamIDs might also
 * be useful.
 *
 */

TEST(MsrpPruningTestGroup, Prune_Multiple_Clients)
{
	static struct sockaddr_in client1;
	static struct sockaddr_in client2;

	memset(&client1, 0, sizeof(client1));
	memset(&client2, 1, sizeof(client2));

	/* no error returned for first client */
	msrp_recv_cmd("S??", strlen("S??") + 1, &client1);
	CHECK(msrp_tests_cmd_ok(test_state.ctl_msg_data));

	/* error returned for second client */
	msrp_recv_cmd("S??", strlen("S??") + 1, &client2);
	CHECK(!msrp_tests_cmd_ok(test_state.ctl_msg_data));
}

