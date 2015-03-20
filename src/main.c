//Copyright 2015 Brian William Denton

#include <gtk/gtk.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "structs.h"

GtkWidget *resourceList;
GtkListStore *listStore;
GtkTreeIter iter;
GtkTreeSelection *selection;

int size=0;
char *resourceMap;
struct resourceHead *resourceMapHead;
struct resourceData *dataIndex;

openFile(GtkWidget *widget,gpointer window)
{
	GtkWidget *openFileDialog;
	FILE *fp=NULL;
	int haveData=0;

	openFileDialog=gtk_file_chooser_dialog_new("Select sounds.map or bitmaps.map?",window,GTK_FILE_CHOOSER_ACTION_OPEN,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,NULL);
	if (gtk_dialog_run(GTK_DIALOG(openFileDialog))==GTK_RESPONSE_ACCEPT)
	{
		char *infile;
		infile=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(openFileDialog));
		fp=fopen(infile,"rb");
		if (fp==NULL)
		{
			gtk_widget_destroy(openFileDialog);
			openFileDialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"File Read Error","Error");
			gtk_dialog_run(GTK_DIALOG(openFileDialog));
			gtk_widget_destroy(openFileDialog);
			return;
		}
		fseek(fp,0,SEEK_END);
		size=ftell(fp);
		fseek(fp,0,SEEK_SET);
		resourceMap=malloc(size);
		fread(resourceMap,size,1,fp);
		//memcpy(&soundMapHead,soundMap,sizeof(soundMapHead));
		resourceMapHead=resourceMap;
		if (resourceMapHead->dataType!=2&&resourceMapHead->dataType!=1)
		{
			fclose(fp);
			free(resourceMap);
			gtk_widget_destroy(openFileDialog);
			openFileDialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"The file is not a correctly formated Halo 1 resource map file","Error");
			gtk_dialog_run(GTK_DIALOG(openFileDialog));
			gtk_widget_destroy(openFileDialog);
			return;
		}
		haveData=1;
	}
	gtk_widget_destroy(openFileDialog);
	if (haveData)
	{
		loadData(window);
	}
	return;
}

loadData(gpointer window)
{
	int resourceIndex=0;
	int pullData;
	int charIndex;
	char dataName[250];
	char oggHead[4]={'O','g','g','S'};

	while (resourceIndex!=resourceMapHead->resourceCount)
	{
		//memcpy(&dataIndex,&soundMap[soundMapHead.indexOffset+(resourceIndex*sizeof(dataIndex))],sizeof(dataIndex));
		dataIndex=&resourceMap[resourceMapHead->indexOffset+(resourceIndex*sizeof(struct resourceData))];
		pullData=1;
		charIndex=0;
		while (pullData)
		{
			dataName[charIndex]=resourceMap[charIndex+(resourceMapHead->namesOffset+dataIndex->resourceName)];
			if (dataName[charIndex]=='\0')
			{
				pullData=0;
			}
			charIndex++;
		}
		gtk_list_store_append(GTK_LIST_STORE(listStore),&iter);
		gtk_list_store_set(listStore,&iter,0,resourceIndex,-1);
		gtk_list_store_set(listStore,&iter,1,dataName,-1);
		if (memcmp(&oggHead,&resourceMap[dataIndex->resourceDataOffset],4)==0)
		{
			gtk_list_store_set(listStore,&iter,2,"ogg",-1);
		}
		resourceIndex++;
	}
}

//playSingle(GtkWidget *widget,gpointer window)
//{
//	GtkWidget *playSingleDialog;
//	short *pcmBuffer;
//	char *rawInput;
//	SNDFILE *sfp;
//	SF_INFO soundInfo;
//	SF_VIRTUAL_IO soundFileVirtual;
//
//	selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(soundList));
//	if (gtk_tree_selection_get_selected(selection,GTK_TREE_MODEL(&listStore),&iter))
//	{
//		int row;
//		gtk_tree_model_get (GTK_TREE_MODEL(listStore),&iter,0,&row,-1);
//		//g_print("%d\n",row);
//		//g_free(row);//causes segfaults, leaking memory without?
//		memcpy(&dataIndex,&soundMap[soundMapHead.indexOffset+(row*sizeof(dataIndex))],sizeof(dataIndex));
//		rawInput=malloc(dataIndex.resourceSize);
//		memcpy(rawInput,&soundMap[dataIndex.resourceDataOffset],dataIndex.resourceSize);
//		//soundFileVirtual.get_filelen=dataIndex.resourceSize;
//		//soundFileVirtual.seek=dataIndex.resourceDataOffset;
//		sf_open_virtual(&soundFileVirtual,SFM_READ,&soundInfo,rawInput);
//		g_print("%d format\n",soundInfo.format);
//		//sfp=sf_open
//		return;
//	}
//	playSingleDialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"Nothing Selected!","Error");
//	gtk_dialog_run(GTK_DIALOG(playSingleDialog));
//	gtk_widget_destroy(playSingleDialog);
//}

exportSingleFile(GtkWidget *widget,gpointer window)
{
	GtkWidget *exportSingleFileDialog;
	FILE *fp=NULL;

	selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(resourceList));
	if (gtk_tree_selection_get_selected(selection,GTK_TREE_MODEL(&listStore),&iter))
	{
		int row;
		gtk_tree_model_get (GTK_TREE_MODEL(listStore),&iter,0,&row,-1);
		//g_print("%d\n",row);
		//g_free(row);//causes segfaults, leaking memory without?
		//memcpy(&dataIndex,&soundMap[soundMapHead.indexOffset+(row*sizeof(struct resourceData))],sizeof(dataIndex));
		dataIndex=&resourceMap[resourceMapHead->indexOffset+(row*sizeof(struct resourceData))];
		exportSingleFileDialog=gtk_file_chooser_dialog_new("Select output File",window,GTK_FILE_CHOOSER_ACTION_SAVE,GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,NULL);
		if (gtk_dialog_run(GTK_DIALOG(exportSingleFileDialog))==GTK_RESPONSE_ACCEPT)
		{
			char *outFile;
			outFile=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(exportSingleFileDialog));
			fp=fopen(outFile,"wb");
			if (fp==NULL)
			{
				gtk_widget_destroy(exportSingleFileDialog);
				exportSingleFileDialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"File Write Error","Error");
				gtk_dialog_run(GTK_DIALOG(exportSingleFileDialog));
				gtk_widget_destroy(exportSingleFileDialog);
				return;
			}
			fwrite(&resourceMap[dataIndex->resourceDataOffset],dataIndex->resourceSize,1,fp);
			fclose(fp);
			gtk_widget_destroy(exportSingleFileDialog);
		}
		return;
	}
	exportSingleFileDialog=gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"Select a single resource to export","Error");
	gtk_dialog_run(GTK_DIALOG(exportSingleFileDialog));
	gtk_widget_destroy(exportSingleFileDialog);
}

destroyAbout(GtkDialog *dialog)
{
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

aboutWindow(GtkWidget *widget,gpointer window)
{
	GtkWidget *haloResourceAbout;
	const gchar *authors[]={"Brian Denton",NULL};

	haloResourceAbout=gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(haloResourceAbout),"halo-resources 0.1");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(haloResourceAbout),"Copyright 2015 Brian Denton");
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(haloResourceAbout),authors);
	gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(haloResourceAbout),"Project Page");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(haloResourceAbout),"https://github.com/mrblarg64/halo-resources");
	gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(haloResourceAbout),GTK_LICENSE_GPL_3_0);
	g_signal_connect(GTK_ABOUT_DIALOG(haloResourceAbout),"response",G_CALLBACK(destroyAbout),NULL);
	gtk_widget_show(haloResourceAbout);
}

main(int argc,char *argv[])
{
	GtkWidget *window, *grid, *menuBar, *fileMenuButton, *fileMenu, *fileOpenButton, *fileExportButton, *fileQuitButton, *helpMenuButton, *helpMenu, *helpAboutButton,*searchBar, *scrollingWindow, *exportSingleButton;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *indexColumn, *tagColumn, *typeColumn;

	gtk_init(&argc,&argv);
	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),"halo-music");
	gtk_window_set_default_size(GTK_WINDOW(window),1000,700);
	grid=gtk_grid_new();
	gtk_widget_set_hexpand(grid,TRUE);
	gtk_widget_set_vexpand(grid,TRUE);
	gtk_container_add(GTK_CONTAINER(window),grid);

//MENU SETUP
	menuBar=gtk_menu_bar_new();
	fileMenu=gtk_menu_new();
	fileMenuButton=gtk_menu_item_new_with_mnemonic("_File");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileMenuButton),fileMenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar),fileMenuButton);

	fileOpenButton=gtk_menu_item_new_with_label("Open");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu),fileOpenButton);

	fileExportButton=gtk_menu_item_new_with_label("Export");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu),fileExportButton);

	fileQuitButton=gtk_menu_item_new_with_label("Quit");
	gtk_menu_shell_append(GTK_MENU_SHELL(fileMenu),fileQuitButton);

	helpMenu=gtk_menu_new();

	helpMenuButton=gtk_menu_item_new_with_mnemonic("_Help");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(helpMenuButton),helpMenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menuBar),helpMenuButton);

	helpAboutButton=gtk_menu_item_new_with_label("About");
	gtk_menu_shell_append(GTK_MENU_SHELL(helpMenu),helpAboutButton);

	gtk_widget_set_hexpand(menuBar,TRUE);

	gtk_grid_attach(GTK_GRID(grid),menuBar,0,0,2,1);

//SCROLLING SETUP
	scrollingWindow=gtk_scrolled_window_new(NULL,NULL);
	gtk_grid_attach(GTK_GRID(grid),scrollingWindow,0,2,2,1);

//TREE VIEW SETUP
	listStore=gtk_list_store_new(3,G_TYPE_INT,G_TYPE_STRING,G_TYPE_STRING);
	resourceList=gtk_tree_view_new_with_model(GTK_TREE_MODEL(listStore));
	renderer=gtk_cell_renderer_text_new();
	indexColumn=gtk_tree_view_column_new_with_attributes("Index In File",renderer,"text",0,NULL);
	tagColumn=gtk_tree_view_column_new_with_attributes("Tag - (Resource Name)",renderer,"text",1,NULL);
	typeColumn=gtk_tree_view_column_new_with_attributes("Data Type",renderer,"text",2,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(resourceList),indexColumn);
	gtk_tree_view_append_column(GTK_TREE_VIEW(resourceList),tagColumn);
	gtk_tree_view_append_column(GTK_TREE_VIEW(resourceList),typeColumn);




	//soundList=gtk_tree_view_new_with_model(GTK_TREE_MODEL(listStore));
	//gtk_tree_view_set_model(GTK_TREE_VIEW(soundList),GTK_TREE_MODEL(listStore));




	gtk_widget_set_hexpand(resourceList,TRUE);
	gtk_widget_set_vexpand(resourceList,TRUE);
	gtk_container_add(GTK_CONTAINER(scrollingWindow),resourceList);

//SEARCH SETUP
	searchBar=gtk_search_entry_new();
	gtk_grid_attach(GTK_GRID(grid),searchBar,0,1,2,1);

//TEMPORARY BOTTOM BUTTONS

	exportSingleButton=gtk_button_new_with_label("Export Selected Single File");
	gtk_grid_attach(GTK_GRID(grid),exportSingleButton,0,3,2,1);

//BOTTOM INFO SETUP

//WINDOW SETUP

	gtk_widget_show_all(window);

	g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(fileQuitButton,"activate",G_CALLBACK(gtk_main_quit),NULL);
	g_signal_connect(fileOpenButton,"activate",G_CALLBACK(openFile),window);
	g_signal_connect(exportSingleButton,"clicked",G_CALLBACK(exportSingleFile),window);
	g_signal_connect(fileExportButton,"activate",G_CALLBACK(exportSingleFile),window);
	g_signal_connect(helpAboutButton,"activate",G_CALLBACK(aboutWindow),window);
	gtk_main();
}