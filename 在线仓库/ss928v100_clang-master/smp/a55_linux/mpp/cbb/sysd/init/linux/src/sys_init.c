/*
 * Copyright (c) 2025 HiSilicon (Shanghai) Technologies Co., Ltd.
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
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/of_platform.h>
#include "ot_common.h"
#include "ot_osal.h"
#include "ot_sys_mod_init.h"

/*
static td_bool g_sys_reg_by_dts = TD_FALSE;
static td_void *g_reg_crg_base_addr = TD_NULL;
static td_void *g_reg_sys_base_addr = TD_NULL;
static td_void *g_reg_ddr0_base_addr = TD_NULL;
static td_void *g_reg_misc_base_addr = TD_NULL;

static void sys_set_base_config(void)
{
    sys_set_reg_by_dts(g_sys_reg_by_dts);
    sys_set_crg_base_addr(g_reg_crg_base_addr);
    sys_set_sys_base_addr(g_reg_sys_base_addr);
    sys_set_ddr0_base_addr(g_reg_ddr0_base_addr);
    sys_set_misc_base_addr(g_reg_misc_base_addr);
}
*/
static int ot_sys_probe(struct platform_device *pdev)
{
    if (pdev == TD_NULL) {
        osal_printk("%s, %d, dev is NULL!!\n", __FUNCTION__, __LINE__);
        return TD_FAILURE;
    }

    // sys_set_base_config();
    if (sys_do_mod_init() != TD_SUCCESS) {
        return TD_FAILURE;
    }

    return 0;
}

static int ot_sys_remove(struct platform_device *pdev)
{
    ot_unused(pdev);

    sys_do_mod_exit();
    return 0;
}

static const struct of_device_id g_ot_sys_match[] = {
    { .compatible = "vendor,sys" },
    {},
};
MODULE_DEVICE_TABLE(of, g_ot_sys_match);

static struct platform_driver g_ot_sys_driver = {
    .probe = ot_sys_probe,
    .remove = ot_sys_remove,
    .driver = {
        .name = "ot_sys",
        .of_match_table = g_ot_sys_match,
    },
};

osal_module_platform_driver(g_ot_sys_driver);

MODULE_LICENSE("Proprietary");
