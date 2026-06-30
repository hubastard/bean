#include "common/app_shell.h"
#include <bean/pdfkit.h>
#include <limits.h>
#include <libgen.h>
#include <mach-o/dyld.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static NSString bean_pdfkit_sample_path(void) {
    static const char* fallbacks[] = {
        "examples/pdfkit/sample.pdf",
        "sample.pdf",
        NULL,
    };
    char exe_path[PATH_MAX];
    uint32_t size = (uint32_t)sizeof(exe_path);

    if (_NSGetExecutablePath(exe_path, &size) == 0) {
        char dir_path[PATH_MAX];
        strncpy(dir_path, exe_path, sizeof(dir_path));
        dir_path[sizeof(dir_path) - 1] = '\0';
        char sample_path[PATH_MAX];
        snprintf(sample_path, sizeof(sample_path), "%s/sample.pdf", dirname(dir_path));
        if (access(sample_path, R_OK) == 0) {
            return bean_str(sample_path);
        }
    }

    for (size_t i = 0; fallbacks[i] != NULL; ++i) {
        if (access(fallbacks[i], R_OK) == 0) {
            return bean_str(fallbacks[i]);
        }
    }

    return bean_str("examples/pdfkit/sample.pdf");
}

int main(void) {
    BEAN_AUTORELEASE_POOL {
        NSString path = bean_pdfkit_sample_path();
        NSURL url = NSURL_FileURLWithPath(path);

        PDFDocument document = (PDFDocument)bean_alloc(bean_cls("PDFDocument"));
        document = (PDFDocument)PDFDocument_InitWithURL(document, url);
        if (!document || PDFDocument_PageCount(document) == 0) {
            const char* path_utf8 = bean_nsstring_to_utf8(path);
            fprintf(stderr, "Failed to load PDF: %s\n", path_utf8 ? path_utf8 : "(null)");
            return 1;
        }

        const char* path_utf8 = bean_nsstring_to_utf8(path);
        printf(
            "PDFKit: loaded %s (%lu page(s))\n",
            path_utf8 ? path_utf8 : "(null)",
            (unsigned long)PDFDocument_PageCount(document));

        NSWindow window = bean_example_window("Bean PDFKit", 640.0, 800.0);
        NSView content = NSWindow_ContentView(window);
        CGRect bounds = NSView_Bounds(content);

        PDFView pdf_view = (PDFView)bean_alloc(bean_cls("PDFView"));
        pdf_view = (PDFView)NSView_InitWithFrame((NSView)pdf_view, bounds);
        PDFView_SetAutoScales(pdf_view, YES);
        PDFView_SetDocument(pdf_view, document);
        bean_example_add_fill_subview(content, (NSView)pdf_view);

        bean_example_run(window);
    }
    return 0;
}
