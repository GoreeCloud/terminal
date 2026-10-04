#include "gc-onboarding.h"

#include "gc-onboarding-state.h"

typedef struct {
    GtkWindow *window;
    GtkLabel *progress_label;
    GtkLabel *title_label;
    GtkLabel *body_label;
    GtkButton *back_button;
    GtkButton *next_button;
    guint step;
    gboolean replay;
    char *state_path;
} OnboardingView;

static const char *page_titles[] = {
    "Welcome to GoreeCloud Terminal",
    "Know where commands will run",
    "Work quickly with tabs and panes",
    "Use portable local profiles and restored tabs",
    "Search and review risky pastes",
    "Current Development safety boundary",
};

static const char *page_bodies[] = {
    "GoreeCloud Terminal currently opens local shell sessions only. The mockup-inspired workspace is a Development presentation, so planned remote, cloud, persistence, profile, and container features are not implied by this build.",
    "The bottom status bar keeps privilege state, user@host, the active working directory, and session status visible. On VTE 0.78+ with compatible shell integration, status can also show command-running state and the last exit code. If that protocol metadata is unavailable, GoreeCloud Terminal keeps the generic session status instead of guessing from prompt text. Treat these indicators as execution context, not as authorization.",
    "Use the + control or Ctrl+Shift+T to open a new tab; each tab also exposes an explicit close control. The Local Shell sidebar mirrors the new-tab, split, and search actions without pretending that remote environments are already configured. Split left/right with Ctrl+Shift+E or top/bottom with Ctrl+Shift+O. Cycle pane focus with Alt+Left and Alt+Right; close the active pane with Ctrl+Shift+X. Tab navigation remains Ctrl+PageUp and Ctrl+PageDown.",
    "The Profile selector can launch local tabs from portable profiles in $XDG_CONFIG_HOME/goreecloud-terminal/profiles.ini. This Development slice supports profile name, shell, working directory, font, foreground/background colors, environment overrides, and bounded scrollback. The reload glyph refreshes the file without restarting. Open tabs restore their profile and working directory from local versioned state after restart; terminal text, command history, running processes, and split-pane layouts are not persisted.",
    "Search the active pane with Ctrl+Shift+F; Regex and Case options are available in the search bar. Clipboard paste through Ctrl+Shift+V or Shift+Insert and primary-selection middle-click paste read the text first and require confirmation when the text contains a line break. Ctrl+click a recognized web link or existing local path to open it with the desktop handler.",
    "Local profiles, per-profile environment overrides, bounded scrollback, and top-level tab restoration are now available, but startup commands, keybindings, split-layout restoration, command-boundary navigation, SSH, containers, and complete Glaze 1.6.0 consumer acceptance remain incomplete. You can replay this guide from Help at any time.",
};

static const guint page_count = G_N_ELEMENTS(page_titles);

static void
view_free(gpointer data)
{
    OnboardingView *view = data;

    g_free(view->state_path);
    g_free(view);
}

static void
persist_progress(OnboardingView *view, gboolean completed)
{
    GcOnboardingState state = {
        .completed = completed,
        .step = view->step,
    };
    GError *error = NULL;

    if (view->replay) {
        return;
    }

    if (!gc_onboarding_state_save(view->state_path, &state, &error)) {
        g_warning("Unable to persist onboarding progress: %s", error->message);
        g_clear_error(&error);
    }
}

static void
render_page(OnboardingView *view)
{
    g_autofree char *progress = g_strdup_printf(
        "Step %u of %u",
        view->step + 1,
        page_count
    );

    gtk_label_set_text(view->progress_label, progress);
    gtk_label_set_text(view->title_label, page_titles[view->step]);
    gtk_label_set_text(view->body_label, page_bodies[view->step]);
    gtk_widget_set_sensitive(GTK_WIDGET(view->back_button), view->step > 0);
    gtk_button_set_label(
        view->next_button,
        view->step + 1 == page_count ? "Finish" : "Next"
    );
}

static void
on_back_clicked(GtkButton *button, gpointer user_data)
{
    OnboardingView *view = user_data;
    (void) button;

    if (view->step == 0) {
        return;
    }

    view->step--;
    persist_progress(view, FALSE);
    render_page(view);
}

static void
on_next_clicked(GtkButton *button, gpointer user_data)
{
    OnboardingView *view = user_data;
    (void) button;

    if (view->step + 1 == page_count) {
        persist_progress(view, TRUE);
        gtk_window_destroy(view->window);
        return;
    }

    view->step++;
    persist_progress(view, FALSE);
    render_page(view);
}

static gboolean
on_close_request(GtkWindow *window, gpointer user_data)
{
    OnboardingView *view = user_data;
    (void) window;

    persist_progress(view, FALSE);
    return FALSE;
}

static void
show_onboarding(GtkWindow *parent, gboolean replay, guint start_step)
{
    OnboardingView *view = g_new0(OnboardingView, 1);
    GtkWidget *window = gtk_window_new();
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    GtkWidget *kicker = gtk_label_new("GOREECLOUD TERMINAL · DEVELOPMENT");
    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);

    view->window = GTK_WINDOW(window);
    view->progress_label = GTK_LABEL(gtk_label_new(NULL));
    view->title_label = GTK_LABEL(gtk_label_new(NULL));
    view->body_label = GTK_LABEL(gtk_label_new(NULL));
    view->back_button = GTK_BUTTON(gtk_button_new_with_label("Back"));
    view->next_button = GTK_BUTTON(gtk_button_new_with_label("Next"));
    view->step = MIN(start_step, page_count - 1);
    view->replay = replay;
    view->state_path = gc_onboarding_state_default_path();

    g_object_set_data_full(
        G_OBJECT(window),
        "goreecloud-onboarding-view",
        view,
        view_free
    );

    gtk_window_set_title(view->window, "Welcome — GoreeCloud Terminal");
    gtk_window_set_default_size(view->window, 600, 390);
    gtk_window_set_resizable(view->window, FALSE);
    gtk_window_set_modal(view->window, TRUE);
    gtk_window_set_transient_for(view->window, parent);

    gtk_widget_add_css_class(root, "onboarding-card");
    gtk_widget_set_margin_top(root, 24);
    gtk_widget_set_margin_bottom(root, 24);
    gtk_widget_set_margin_start(root, 28);
    gtk_widget_set_margin_end(root, 28);

    gtk_widget_add_css_class(kicker, "onboarding-kicker");
    gtk_label_set_xalign(GTK_LABEL(kicker), 0.0f);
    gtk_label_set_xalign(view->progress_label, 0.0f);
    gtk_widget_add_css_class(GTK_WIDGET(view->title_label), "onboarding-title");
    gtk_label_set_xalign(view->title_label, 0.0f);
    gtk_label_set_wrap(view->title_label, TRUE);

    gtk_widget_add_css_class(GTK_WIDGET(view->body_label), "onboarding-body");
    gtk_label_set_xalign(view->body_label, 0.0f);
    gtk_label_set_wrap(view->body_label, TRUE);
    gtk_label_set_justify(view->body_label, GTK_JUSTIFY_LEFT);
    gtk_widget_set_vexpand(GTK_WIDGET(view->body_label), TRUE);

    gtk_widget_set_hexpand(buttons, TRUE);
    gtk_widget_set_halign(buttons, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(buttons), GTK_WIDGET(view->back_button));
    gtk_box_append(GTK_BOX(buttons), GTK_WIDGET(view->next_button));

    gtk_box_append(GTK_BOX(root), kicker);
    gtk_box_append(GTK_BOX(root), GTK_WIDGET(view->progress_label));
    gtk_box_append(GTK_BOX(root), GTK_WIDGET(view->title_label));
    gtk_box_append(GTK_BOX(root), GTK_WIDGET(view->body_label));
    gtk_box_append(GTK_BOX(root), buttons);

    gtk_window_set_child(view->window, root);

    g_signal_connect(
        view->back_button,
        "clicked",
        G_CALLBACK(on_back_clicked),
        view
    );
    g_signal_connect(
        view->next_button,
        "clicked",
        G_CALLBACK(on_next_clicked),
        view
    );
    g_signal_connect(
        view->window,
        "close-request",
        G_CALLBACK(on_close_request),
        view
    );

    render_page(view);
    gtk_window_present(view->window);
}

void
gc_onboarding_maybe_show(GtkWindow *parent)
{
    GcOnboardingState state;
    g_autofree char *path = gc_onboarding_state_default_path();
    GError *error = NULL;

    if (!gc_onboarding_state_load(path, &state, &error)) {
        g_warning("Unable to read onboarding state: %s", error->message);
        g_clear_error(&error);
        gc_onboarding_state_init(&state);
    }

    if (!state.completed) {
        show_onboarding(parent, FALSE, state.step);
    }
}

void
gc_onboarding_show_replay(GtkWindow *parent)
{
    show_onboarding(parent, TRUE, 0);
}
