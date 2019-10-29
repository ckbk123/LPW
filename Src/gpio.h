
#define LED_PORT GPIOA
#define LED_PIN LL_GPIO_PIN_5
#define BUT_PORT GPIOC
#define BUT_PIN LL_GPIO_PIN_13
#define OUT_50HZ_PORT GPIOC
#define OUT_50HZ_PIN LL_GPIO_PIN_10

// config
void GPIO_init(void);

// utilisation
void LED_GREEN( int val );
int BLUE_BUTTON();
