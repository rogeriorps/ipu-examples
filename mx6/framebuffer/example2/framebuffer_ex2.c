

#include <stdio.h>
#include <ncurses.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <math.h>
#include <linux/mxcfb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define WIDTH 30
#define HEIGHT 10 

int startx = 0;
int starty = 0;

char *choices[] = { 
			"Brightness ",
			"Contrast   ",
			"Saturation ",
			"Hue        ",
		  };

char *min[] = { 
			"0%",
			"0%",
			"0%",
			"0%",
		  };

char *max[] = { 
			"100%",
			"100%",
			"100%",
			"100%",
		  };

int cur_value[] = { 
			50,
			50,
			50,
			50,
		  };

static const int ycbcr2rgb_coeff_sd[5][3] = {
	{0x095, 0x000, 0x0CC},
	{0x095, 0x3CE, 0x398},
	{0x095, 0x0FF, 0x000},
	{0x3E42, 0x010A, 0x3DD6},	/*B0,B1,B2 */
	{0x1, 0x1, 0x1},	/*S0,S1,S2 */
};

static const int ycbcr2rgb_coeff_hd[5][3] = {
	{0x095, 0x000, 0x0E5},
	{0x095, 0x3E5, 0x3BC},
	{0x095, 0x10E, 0x000},
	{0x3E10, 0x0099, 0x3DBE},	/*B0,B1,B2 */
	{0x1, 0x1, 0x1},	/*S0,S1,S2 */
};

/* */
static double default_matrix_coeff_sd[3][3] = {
		{  1.1644,  0.0000,  1.5962 },
		{  1.1644, -0.3937, -0.8133 },
		{  1.1644,  2.0174,  0.0000 },
};

static double default_matrix_coeff_hd[3][3] = {
		{  1.1644,  0.0000,  1.7927 },
		{  1.1644, -0.2132, -0.5329 },
		{  1.1644,  2.1124,  0.0000 },
};

static double brightness_saturation_matrix[3][3] = {
		{  1.0000,  0.0000,  0.0000 },
		{  0.0000,  1.0000,  0.0000 },
		{  0.0000,  0.0000,  1.0000 },
};

static double hue_matrix[3][3] = {
		{  1.0000,  0.0000,  0.0000 },
		{  0.0000,  1.0000,  0.0000 },
		{  0.0000,  0.0000,  1.0000 },
};

static double SrSgSb_matrix[3][3] = {
		{  1.0000,  0.0000,  0.0000 },
		{  0.0000,  1.0000,  0.0000 },
		{  0.0000,  0.0000,  1.0000 },
};

static double ColorTemp_coeff_matrix[3][3] = {
		{  1.0000,  0.0000,  0.0000 },
		{  0.0000,  1.0000,  0.0000 },
		{  0.0000,  0.0000,  1.0000 },
};

static double ColorTemp_table[3][3];
static double White_P0[3] = { 1.0,1.0,1.0 };
static double White_P1[3] = { 1.0,1.0,1.0 };
static double White_XYZp0[3] = { 1.0,1.0,1.0 };
static double White_XYZp1[3] = { 1.0,1.0,1.0 };

static int enable_color_temp = 0;

static double xyz_inverse_matrix[3][3];
static double XYZ_matrix[3][3];
static double XYZ_inverse_matrix[3][3];

static double KRGB[3];
static double Kmax = 0;

static double color_correction_matrix[5][3];

static int file_matrix[5][3];

static int brightness = 50;
static int contrast = 50;
static int saturation = 50;
static int hue = 50;

static double brightness_coeff = 1.0;
static double contrast_coeff = 1.0;
static double saturation_coeff = 1.0;
static double hue_coeff = 0;

static double r_gamma = 1.0;
static double g_gamma = 1.0;
static double b_gamma = 1.0;

#define PI 3.141592

int n_choices = sizeof(choices) / sizeof(char *);
void print_menu(WINDOW *menu_win, int highlight);

void create_color_correction_matrix()
{
	int i,j;
	
	double tmp;

	for(i = 0;i < 3;i++)
	{
		for(j = 0;j < 3;j++)
		{
			color_correction_matrix[i][j] = color_correction_matrix[i][j] * contrast_coeff;
		}
	}

	brightness_saturation_matrix[0][0] = brightness_coeff;
	brightness_saturation_matrix[1][1] = saturation_coeff;
	brightness_saturation_matrix[2][2] = saturation_coeff;

	for(i = 0;i < 3;i++)
	{
		for(j = 0;j < 3;j++)
		{
			color_correction_matrix[i][j] = color_correction_matrix[i][j] * brightness_saturation_matrix[j][j];
		}
	}

	hue_matrix[1][1] = cos(hue_coeff * PI / 180.);
	hue_matrix[1][2] = sin(hue_coeff * PI / 180.);
	hue_matrix[2][1] = sin(hue_coeff * PI / 180.) * -1.;
	hue_matrix[2][2] = hue_matrix[1][1];

	for(i = 0;i < 3;i++)
	{
		color_correction_matrix[i][0] = color_correction_matrix[i][0];
	
		tmp =  color_correction_matrix[i][1];

		color_correction_matrix[i][1] = ( tmp * hue_matrix[1][1] ) + ( color_correction_matrix[i][2] * hue_matrix[2][1] );
		color_correction_matrix[i][2] = ( tmp * hue_matrix[1][2] ) + ( color_correction_matrix[i][2] * hue_matrix[2][2] );
	}
	

	for(i = 0;i < 3;i++)
	{
		color_correction_matrix[3][i] = (color_correction_matrix[i][0] * -16.) + (color_correction_matrix[i][1] * -128.) + (color_correction_matrix[i][2] * -128.);
	}

	for(i = 0;i < 3;i++)
	{
		color_correction_matrix[3][i] = color_correction_matrix[3][i] * 2.0;
	}

	for(i = 0;i < 3;i++)
	{
		for(j = 0;j < 3;j++)
		{
			color_correction_matrix[i][j] = color_correction_matrix[i][j] * 128.;
		}
	}

	color_correction_matrix[4][0] = 1.;
	color_correction_matrix[4][1] = 1.;
	color_correction_matrix[4][2] = 1.;
}

void fb_set_std_table(int fd_fb,int index)
{
	struct mxcfb_csc_matrix csc_matrix;
	int retval;

	memset(&csc_matrix,0,sizeof(csc_matrix));

	if(index == 0)
	{
		// BT709 standard
		memcpy(csc_matrix.param,ycbcr2rgb_coeff_hd,sizeof(ycbcr2rgb_coeff_hd));
	
	}
	else if(index == 1)
	{
		// BT601 standard
		memcpy(csc_matrix.param,ycbcr2rgb_coeff_sd,sizeof(ycbcr2rgb_coeff_sd));
	}
	else if(index == 2)
	{
		int i,j;
		
		for(i = 0;i < 3;i++)
		{
			for(j = 0;j < 3;j++)
			{
				csc_matrix.param[i][j] = file_matrix[i][j] & 0x3FF;
			}
		}
		for(i = 0;i < 3;i++)
		{
			csc_matrix.param[3][i] = file_matrix[3][i] & 0x3FFF;
			csc_matrix.param[4][i] = file_matrix[4][i];
		}
	}
	else
	{
		int i,j;
		
		for(i = 0;i < 3;i++)
		{
			for(j = 0;j < 3;j++)
			{
				csc_matrix.param[i][j] = (int)color_correction_matrix[i][j] & 0x3FF;
			}
		}
		for(i = 0;i < 3;i++)
		{
			csc_matrix.param[3][i] = (int)color_correction_matrix[3][i] & 0x3FFF;
			csc_matrix.param[4][i] = (int)color_correction_matrix[4][i];
		}

	}

/*	printf("\n");

	printf("CSC parameters\n");

	printf("\n");

	printf("-------------------------------\n");
	printf("Hex. table\n");
	printf("-------------------------------\n");
	printf("{ %03X, %03X, %03X }\n",csc_matrix.param[0][0],csc_matrix.param[0][1],csc_matrix.param[0][2]);
	printf("{ %03X, %03X, %03X }\n",csc_matrix.param[1][0],csc_matrix.param[1][1],csc_matrix.param[1][2]);
	printf("{ %03X, %03X, %03X }\n",csc_matrix.param[2][0],csc_matrix.param[2][1],csc_matrix.param[2][2]);
	printf("{ %04X, %04X, %04X }\n",csc_matrix.param[3][0],csc_matrix.param[3][1],csc_matrix.param[3][2]);
	printf("{ %03X, %03X, %03X }\n",csc_matrix.param[4][0],csc_matrix.param[4][1],csc_matrix.param[4][2]);
*/
	retval = ioctl(fd_fb, MXCFB_CSC_UPDATE, &csc_matrix);
	if (retval < 0) {
		printf("Ioctl MXCFB_CSC_UPDATE fail!\n");
	}
}

int main()
{	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int c;
	int fd;

	fd = open("/dev/fb1",O_RDWR);
	if(fd < 0)
	{
		printf("Can not open device\n");
		return -1;
	}

	initscr();
	clear();
	noecho();
	cbreak();	/* Line buffering disabled. pass on everything */
	startx = (80 - WIDTH) / 2;
	starty = (24 - HEIGHT) / 2;
		
	menu_win = newwin(HEIGHT, WIDTH, starty, startx);
	keypad(menu_win, TRUE);
	mvprintw(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
	refresh();
	print_menu(menu_win, highlight);
	while(1)
	{	c = wgetch(menu_win);
		switch(c)
		{	case KEY_UP:
				if(highlight == 1)
					highlight = n_choices;
				else
					--highlight;
				break;
			case KEY_DOWN:
				if(highlight == n_choices)
					highlight = 1;
				else 
					++highlight;
				break;
			case KEY_RIGHT:
				if (cur_value[highlight-1]<100)
					cur_value[highlight-1]++;
				break;
			case KEY_LEFT:
				if (cur_value[highlight-1]>0)
					cur_value[highlight-1]--;
				break;
			case 10:
				choice = highlight;
				break;
			default:
				mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", c, c);
				refresh();
				break;
		}

		
		brightness = cur_value[0];
		brightness_coeff = 2.0 * ((float)brightness/100.);

		contrast = cur_value[1];
		contrast_coeff = 2.0 * ((float)contrast/100.);

		saturation = cur_value[2];
		saturation_coeff = 2.0 * ((float)saturation/100.);

		hue = cur_value[3];
		hue_coeff = 180 + (180 * ((float)hue/100.));

		memcpy(color_correction_matrix,default_matrix_coeff_hd,sizeof(default_matrix_coeff_hd));
		create_color_correction_matrix();
		fb_set_std_table(fd, 255);
		
		print_menu(menu_win, highlight);
		if(choice != 0)	/* User did a choice come out of the infinite loop */
			break;
	}	
	mvprintw(23, 0, "You chose choice %d with choice string %s\n", choice, choices[choice - 1]);
	clrtoeol();
	refresh();
	endwin();
	return 0;
}


void print_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;	

	x = 2;
	y = 2;
	box(menu_win, 0, 0);
	for(i = 0; i < n_choices; ++i)
	{	if(highlight == i + 1) /* High light the present choice */
		{	wattron(menu_win, A_REVERSE); 
			mvwprintw(menu_win, y, x, "%s  - %s %d %s", choices[i], min[i], cur_value[i], max[i]);
			wattroff(menu_win, A_REVERSE);
		}
		else
			mvwprintw(menu_win, y, x, "%s  - %s %d %s", choices[i], min[i], cur_value[i], max[i]);
		++y;
	}
	wrefresh(menu_win);
}

