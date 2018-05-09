#include <zephyr/types.h>
#include <stddef.h>
#include <sys/types.h>
#include <device.h>

#if defined(CONFIG_FLASH_PAGE_LAYOUT)
struct flash_pages_layout {
	size_t pages_count; /* count of pages sequence of the same size */
	size_t pages_size;
};
#endif /* CONFIG_FLASH_PAGE_LAYOUT */


typedef int (*flash_api_read)(struct device *dev, off_t offset, void *data,
			      size_t len);
typedef int (*flash_api_write)(struct device *dev, off_t offset,
			       const void *data, size_t len);
typedef int (*flash_api_erase)(struct device *dev, off_t offset, size_t size);
#if defined(CONFIG_FLASH_PAGE_LAYOUT)

typedef void (*flash_api_pages_layout)(struct device *dev,
				       const struct flash_pages_layout **layout,
				       size_t *layout_size);
#endif /* CONFIG_FLASH_PAGE_LAYOUT */

struct flash_driver_api {
	flash_api_read read;
	flash_api_write write;
	flash_api_erase erase;
#if defined(CONFIG_FLASH_PAGE_LAYOUT)
	flash_api_pages_layout page_layout;
#endif /* CONFIG_FLASH_PAGE_LAYOUT */
	const size_t write_block_size;
};

