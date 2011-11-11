/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _MPC83XX_GPIO_H_
#define _MPC83XX_GPIO_H_

/*
 * The MCP83xx's 1-2 GPIO controllers each with 32 bits.
 */
#if defined(CONFIG_MPC8313) || defined(CONFIG_MPC8308) || \
	defined(CONFIG_MPC8315)
#define MPC83XX_GPIO_CTRLRS 1
#elif defined(CONFIG_MPC834x) || defined(CONFIG_MPC837x)
#define MPC83XX_GPIO_CTRLRS 2
#else
#define MPC83XX_GPIO_CTRLRS 0
#endif

#define MAX_NUM_GPIOS (32 * MPC83XX_GPIO_CTRLRS)

void mpc83xx_gpio_init_f(void);
void mpc83xx_gpio_init_r(void);

#endif	/* MPC83XX_GPIO_H_ */
