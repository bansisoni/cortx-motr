/* -*- C -*- */
/*
 * Copyright (c) 2012-2020 Seagate Technology LLC and/or its Affiliates
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For any questions about this software or licensing,
 * please email opensource@seagate.com or cortx-questions@seagate.com.
 *
 */


#define M0_TRACE_SUBSYSTEM M0_TRACE_SUBSYS_SNSCM
#include "lib/trace.h"

#include "lib/memory.h"
#include "lib/assert.h"
#include "lib/errno.h"
#include "lib/misc.h"
#include "lib/finject.h"

#include "reqh/reqh_service.h"
#include "sns/cm/trigger_fop.h"
#include "sns/cm/service.h"
#include "sns/cm/cm.h"
#include "sns/cm/sns_cp_onwire.h"

/**
  @defgroup SNSCMSVC SNS copy machine service
  @ingroup SNSCM

  @{
*/

/** Copy machine service type operations.*/
static int rebalance_svc_allocate(struct m0_reqh_service **service,
				  const struct m0_reqh_service_type *stype);

static const struct m0_reqh_service_type_ops rebalance_svc_type_ops = {
	.rsto_service_allocate = rebalance_svc_allocate
};

M0_CM_TYPE_DECLARE(sns_rebalance, M0_CM_REBALANCE_OPCODE,
		   &rebalance_svc_type_ops, "M0_CST_SNS_REB", M0_CST_SNS_REB);

/** Copy machine service operations.*/
static int rebalance_svc_start(struct m0_reqh_service *service);
static void rebalance_svc_stop(struct m0_reqh_service *service);

static const struct m0_reqh_service_ops rebalance_svc_ops = {
	.rso_start	 = rebalance_svc_start,
	.rso_start_async = m0_reqh_service_async_start_simple,
	.rso_stop	 = rebalance_svc_stop,
	.rso_fini	 = m0_sns_cm_svc_fini
};

extern const struct m0_cm_ops sns_rebalance_ops;
extern const struct m0_fom_type_ops rebalance_cp_fom_type_ops;

M0_INTERNAL void m0_sns_cm_rebalance_cpx_init(void);
M0_INTERNAL void m0_sns_cm_rebalance_cpx_fini(void);

/**
 * Allocates and initialises SNS copy machine.
 * This allocates struct m0_sns_cm and invokes m0_cm_init() to initialise
 * m0_sns_cm::rc_base.
 */
static int rebalance_svc_allocate(struct m0_reqh_service **service,
				  const struct m0_reqh_service_type *stype)
{
	M0_ENTRY("stype: %p", stype);
	M0_PRE(service != NULL && stype != NULL);
	return M0_RC(m0_sns_cm_svc_allocate(service, stype, &rebalance_svc_ops,
					    &sns_rebalance_ops));
}

static int rebalance_svc_start(struct m0_reqh_service *service)
{
	int rc;

	rc = m0_sns_cm_svc_start(service);
	if (rc == 0) {
		m0_cm_cp_init(&sns_rebalance_cmt, &rebalance_cp_fom_type_ops);
		m0_sns_cm_rebalance_cpx_init();
		m0_sns_cm_rebalance_sw_onwire_fop_init();
		m0_sns_cm_rebalance_trigger_fop_init();
	}
	return M0_RC(rc);
}

static void rebalance_svc_stop(struct m0_reqh_service *service)
{
	m0_sns_cm_svc_stop(service);
	m0_sns_cm_rebalance_cpx_fini();
	m0_sns_cm_rebalance_sw_onwire_fop_fini();
	m0_sns_cm_rebalance_trigger_fop_fini();
}

/** @} SNSCMSVC */
#undef M0_TRACE_SUBSYSTEM

/*
 *  Local variables:
 *  c-indentation-style: "K&R"
 *  c-basic-offset: 8
 *  tab-width: 8
 *  fill-column: 80
 *  scroll-step: 1
 *  End:
 */
