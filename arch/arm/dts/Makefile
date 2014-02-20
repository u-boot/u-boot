dtb-$(CONFIG_EXYNOS5) += exynos5250-arndale.dtb \
	exynos5250-snow.dtb \
	exynos5250-smdk5250.dtb \
	exynos5420-smdk5420.dtb
dtb-$(CONFIG_TEGRA) += tegra20-harmony.dtb \
	tegra20-medcom-wide.dtb \
	tegra20-paz00.dtb \
	tegra20-plutux.dtb \
	tegra20-seaboard.dtb \
	tegra20-tec.dtb \
	tegra20-trimslice.dtb \
	tegra20-ventana.dtb \
	tegra20-whistler.dtb \
	tegra20-colibri_t20_iris.dtb \
	tegra30-beaver.dtb \
	tegra30-cardhu.dtb \
	tegra30-tec-ng.dtb \
	tegra114-dalmore.dtb \
	tegra124-venice2.dtb
dtb-$(CONFIG_ZYNQ) += zynq-zc702.dtb \
	zynq-zc706.dtb \
	zynq-zed.dtb \
	zynq-microzed.dtb \
	zynq-zc770-xm010.dtb \
	zynq-zc770-xm012.dtb \
	zynq-zc770-xm013.dtb

targets += $(dtb-y)

DTC_FLAGS += -R 4 -p 0x1000

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
