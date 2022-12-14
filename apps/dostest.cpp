// ========================================================
//
// FILE:			/apps/test.cpp
//
// AUTHORS:			Jaycie Ewald
//
// PROJECT:			libcohost
//
// LICENSE:			ACSL v1.4
//
// DESCRIPTION:		ImTui Test App
//
// LAST EDITED:		November 26th, 2022
//
// ========================================================

// Determine which version to build
#if !defined(SDL)
#define TERMINAL
#endif

// Standard headers
#include <iostream>

// SDL target headers
#if defined(SDL)
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#endif

// Terminal target headers
#if defined(TERMINAL)
#include "imtui/imtui.h"
#include "imtui/imtui-impl-ncurses.h"
#endif

#if defined(SDL)

#define FONT_WIDTH ImGui::GetFontSize() * 0.5f
#define FONT_HEIGHT ImGui::GetFontSize()

ImGuiIO &StartupSDL(SDL_Window **window, SDL_GLContext *context)
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		exit(-1);
	}

	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Window* win = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
	SDL_GLContext ctx = SDL_GL_CreateContext(win);
	SDL_GL_MakeCurrent(win, ctx);
	SDL_GL_SetSwapInterval(1);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(win, ctx);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Return values
	*window = win;
	*context = ctx;
	return io;
}

void ShutdownSDL(SDL_Window *window, SDL_GLContext context)
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void RenderSDL(SDL_Window *window, ImGuiIO &io, ImVec4 clear_color)
{
	// Rendering
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}

void NewFrameSDL(SDL_Window *window)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
}

void PollEventsSDL(bool *running, SDL_Window *window)
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT)
			*running = false;
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
			*running = false;
	}
}

#endif

#if defined(TERMINAL)

#define FONT_WIDTH 1
#define FONT_HEIGHT 1

// Initialize
ImTui::TScreen *StartupTUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImTui::TScreen *window = ImTui_ImplNcurses_Init(true);
	ImTui_ImplText_Init();

	return window;
}

// Shutdown
void ShutdownTUI()
{
	ImTui_ImplText_Shutdown();
	ImTui_ImplNcurses_Shutdown();
}

// Start a new frame
void NewFrameTUI()
{
	ImTui_ImplNcurses_NewFrame();
	ImTui_ImplText_NewFrame();
	ImGui::NewFrame();
}

// Render frame
void RenderTUI(ImTui::TScreen *window)
{
	ImGui::Render();
	ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), window);
	ImTui_ImplNcurses_DrawScreen();
}

#endif

// Entry point
int main(int argc, char *argv[])
{
	// Variables
	bool b_running = true;
	bool b_logged_in = false;
	bool b_logging_in = false;
	char email[64];
	char password[64];

	#if defined(SDL)
	SDL_Window *window;
	SDL_GLContext gl_context;
	ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	ImGuiIO &io = StartupSDL(&window, &gl_context);
	float font_scale = 2.0f;
	#endif
	#if defined(TERMINAL)
	ImTui::TScreen *window = StartupTUI();
	#endif

	while (b_running)
	{
		// Start new frame & poll events
		#if defined(SDL)
		PollEventsSDL(&b_running, window);
		NewFrameSDL(window);
		ImGui::GetIO().FontGlobalScale = font_scale;
		#endif
		#if defined(TERMINAL)
		NewFrameTUI();
		#endif

		// Main window
		{
			// Set window properties
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

			// Begin window
			if (!ImGui::Begin("Title", &b_running, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
				break;

			// Menubar
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Actions "))
				{
					if (ImGui::MenuItem(" Login ", "begin new session", nullptr, !b_logged_in)) b_logging_in = true;
					ImGui::MenuItem(" Post ", "chost like a champ", nullptr, b_logged_in);

					ImGui::NewLine();
					if (ImGui::MenuItem(" Quit ", "stop chosting")) b_running = false;
					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			// Text
			if (ImGui::BeginTable("homepage", 1))
			{
				ImGui::TableSetupColumn(" Settings ", ImGuiTableColumnFlags_WidthFixed, 16.0f * FONT_WIDTH);

				ImGui::TableNextColumn();

				ImGui::NewLine();
				ImGui::Button(" Notifs ");
				ImGui::NewLine();
				ImGui::Button(" Bookmarks ");
				ImGui::NewLine();
				ImGui::Button(" Search ");
				ImGui::NewLine();
				ImGui::Button(" Profile ");
				ImGui::NewLine();
				ImGui::Button(" Drafts ");
				ImGui::NewLine();
				ImGui::Button(" Following ");
				ImGui::NewLine();
				ImGui::Button(" Followers ");
				ImGui::NewLine();
				ImGui::Button(" Settings ");

				#if defined(SDL)
				ImGui::NewLine();
				ImGui::Text("Font Scale:");
				ImGui::SliderFloat("##fontscale", &font_scale, 1.0f, 2.0f);
				#endif

				ImGui::EndTable();
			}

			// End ImGui processing
			ImGui::End();
		}

		// Timeline window
		{
			// Set window properties
			ImGui::SetNextWindowPos(ImVec2(16 * FONT_WIDTH, 3 * FONT_HEIGHT), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - (17 * FONT_WIDTH), ImGui::GetIO().DisplaySize.y - (3 * FONT_HEIGHT)), ImGuiCond_Always);

			// Begin window
			if (!ImGui::Begin("Timeline", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
				return EXIT_FAILURE;

			// Window elements
			ImGui::NewLine();
			ImGui::TextWrapped("Im trying to explain how i came to the understanding that this reality here on earth is truly a matrix and that there is a reptilian race from the constellation of astro-world who are controlling virtual reality here on earth.");
			ImGui::NewLine();
			ImGui::TextWrapped("reincarnation here has nothing to do with our spiritual growth from cradle to grave and beyond and were never going to get out of this situation without planet x. planet x is not a catastrophe, it is a prison break.");
			ImGui::NewLine();
			ImGui::TextWrapped("the force of this planet as it tears apart the electro-magnetic force field that surrounds the earth will finally reveal the matrix and everyone is going to see it; there will be no doubt, there will be no fighting over belief systems, we are going to have all of the masks pulled away. that is the future for us with planet x.");
			ImGui::NewLine();
			ImGui::TextWrapped("Im trying to explain how i came to the understanding that this reality here on earth is truly a matrix and that there is a reptilian race from the constellation of astro-world who are controlling virtual reality here on earth.");
			ImGui::NewLine();
			ImGui::TextWrapped("reincarnation here has nothing to do with our spiritual growth from cradle to grave and beyond and were never going to get out of this situation without planet x. planet x is not a catastrophe, it is a prison break.");
			ImGui::NewLine();
			ImGui::TextWrapped("the force of this planet as it tears apart the electro-magnetic force field that surrounds the earth will finally reveal the matrix and everyone is going to see it; there will be no doubt, there will be no fighting over belief systems, we are going to have all of the masks pulled away. that is the future for us with planet x.");

			// End window
			ImGui::End();
		}

		// Login window
		if (b_logging_in == true && b_logged_in == false)
		{
			// Set window properties
			ImGui::SetNextWindowPos(ImVec2(4 * FONT_WIDTH, 4 * FONT_HEIGHT), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(48 * FONT_WIDTH, 10 * FONT_HEIGHT), ImGuiCond_Once);

			// Begin window
			if (!ImGui::Begin("Login", nullptr, ImGuiWindowFlags_NoCollapse))
				return EXIT_FAILURE;

			// Window elements
			ImGui::NewLine();
			ImGui::Text("Please enter your email and password:");
			ImGui::NewLine();
			ImGui::InputText("Email", email, IM_ARRAYSIZE(email), ImGuiInputTextFlags_None);
			ImGui::NewLine();
			ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_CharsNoBlank);
			ImGui::NewLine();

			// Login button
			if (ImGui::Button(" Login ") && strlen(password) > 0 && strlen(email) > 0)
				b_logged_in = true;

			// End window
			ImGui::End();
		}

		// Render result
		#if defined(SDL)
		RenderSDL(window, io, clear_color);
		#endif
		#if defined(TERMINAL)
		RenderTUI(window);
		#endif
	}

	// Shutdown
	#if defined(SDL)
	ShutdownSDL(window, gl_context);
	#endif
	#if defined(TERMINAL)
	ShutdownTUI();
	#endif

	return EXIT_SUCCESS;
}
