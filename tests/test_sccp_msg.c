/*
 * Unit tests for the SCCP message serializer and deserializer (sccp_msg.c).
 *
 * Build and run with "make check". Only the Asterisk headers are needed, the
 * few Asterisk functions used by sccp_msg.c are stubbed below.
 */
#include <asterisk.h>
#include <asterisk/localtime.h>
#include <asterisk/logger.h>
#include <asterisk/utils.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../sccp_msg.h"
#include "../sccp_utils.h"

/* wire sizes of the message bodies, from the SCCP protocol */
_Static_assert(sizeof(struct display_prompt_status_message) == 44, "display prompt status size");
_Static_assert(sizeof(struct clear_prompt_status_message) == 8, "clear prompt status size");
_Static_assert(sizeof(struct hookflash_message) == 8, "hookflash size");
_Static_assert(sizeof(struct set_ringer_message) == 20, "set ringer size");
_Static_assert(offsetof(struct button_template_res_message, definition) == 12, "button template header size");
_Static_assert(sizeof(struct button_definition) == 2, "button definition size");
_Static_assert(sizeof(struct button_template_res_message) == 12 + 2 * MAX_BUTTON_DEFINITION, "button template size");

static int failures;
static int checks;

#define CHECK(cond) do { \
	checks++; \
	if (!(cond)) { \
		failures++; \
		fprintf(stderr, "%s:%d: %s: check failed: %s\n", __FILE__, __LINE__, __func__, #cond); \
	} \
} while (0)

/* stubs for the Asterisk functions used by sccp_msg.c */
void ast_log(int level, const char *file, int line, const char *function, const char *fmt, ...)
{
}

struct ast_tm *ast_localtime(const struct timeval *timep, struct ast_tm *p_tm, const char *zone)
{
	memset(p_tm, 0, sizeof(*p_tm));
	return p_tm;
}

static void test_button_template_res_legacy_size(void)
{
	struct sccp_msg msg;
	struct button_definition definition[3] = {
		{1, BT_LINE},
		{2, BT_LINE},
		{3, BT_FEATUREBUTTON},
	};
	size_t i;

	sccp_msg_button_template_res(&msg, definition, 3);

	CHECK(letohl(msg.id) == BUTTON_TEMPLATE_RES_MESSAGE);
	/* 42 buttons on the wire when that's enough, as before the 56 buttons support */
	CHECK(letohl(msg.length) == SCCP_MSG_LEN_FROM_DATA_LEN(12 + 2 * LEGACY_MAX_BUTTON_DEFINITION));
	CHECK(letohl(msg.data.buttontemplate.buttonOffset) == 0);
	CHECK(letohl(msg.data.buttontemplate.buttonCount) == 3);
	CHECK(letohl(msg.data.buttontemplate.totalButtonCount) == 3);
	CHECK(msg.data.buttontemplate.definition[1].lineInstance == 2);
	CHECK(msg.data.buttontemplate.definition[1].buttonDefinition == BT_LINE);
	CHECK(msg.data.buttontemplate.definition[2].buttonDefinition == BT_FEATUREBUTTON);
	for (i = 3; i < LEGACY_MAX_BUTTON_DEFINITION; i++) {
		CHECK(msg.data.buttontemplate.definition[i].buttonDefinition == BT_NONE);
	}
}

static void test_button_template_res_large(void)
{
	struct sccp_msg msg;
	struct button_definition definition[MAX_BUTTON_DEFINITION + 4];
	size_t i;

	for (i = 0; i < ARRAY_LEN(definition); i++) {
		definition[i].lineInstance = i + 1;
		definition[i].buttonDefinition = BT_FEATUREBUTTON;
	}

	/* 7975 with two 24 buttons KEM */
	sccp_msg_button_template_res(&msg, definition, 50);

	CHECK(letohl(msg.length) == SCCP_MSG_LEN_FROM_DATA_LEN(12 + 2 * MAX_BUTTON_DEFINITION));
	CHECK(letohl(msg.data.buttontemplate.buttonCount) == 50);
	CHECK(msg.data.buttontemplate.definition[49].lineInstance == 50);
	CHECK(msg.data.buttontemplate.definition[50].buttonDefinition == BT_NONE);
	CHECK(msg.data.buttontemplate.definition[MAX_BUTTON_DEFINITION - 1].buttonDefinition == BT_NONE);

	/* more buttons than the message can hold are dropped */
	sccp_msg_button_template_res(&msg, definition, ARRAY_LEN(definition));

	CHECK(letohl(msg.data.buttontemplate.buttonCount) == MAX_BUTTON_DEFINITION);
	CHECK(letohl(msg.data.buttontemplate.totalButtonCount) == MAX_BUTTON_DEFINITION);
	CHECK(msg.data.buttontemplate.definition[MAX_BUTTON_DEFINITION - 1].lineInstance == MAX_BUTTON_DEFINITION);
}

static void test_display_prompt_status(void)
{
	struct sccp_msg msg;
	const char *long_text = "0123456789012345678901234567890123456789";

	sccp_msg_display_prompt_status(&msg, "DND", 2, 1234);

	CHECK(letohl(msg.id) == DISPLAY_PROMPT_STATUS_MESSAGE);
	CHECK(letohl(msg.length) == SCCP_MSG_LEN_FROM_DATA_LEN(44));
	CHECK(letohl(msg.data.displayprompt.messageTimeout) == 0);
	CHECK(!strcmp(msg.data.displayprompt.promptMessage, "DND"));
	CHECK(letohl(msg.data.displayprompt.lineInstance) == 2);
	CHECK(letohl(msg.data.displayprompt.callInstance) == 1234);

	/* the text is truncated and always NUL terminated */
	sccp_msg_display_prompt_status(&msg, long_text, 0, 0);

	CHECK(strlen(msg.data.displayprompt.promptMessage) == sizeof(msg.data.displayprompt.promptMessage) - 1);
	CHECK(!strncmp(msg.data.displayprompt.promptMessage, long_text, sizeof(msg.data.displayprompt.promptMessage) - 1));
}

static void test_clear_prompt_status(void)
{
	struct sccp_msg msg;

	sccp_msg_clear_prompt_status(&msg, 1, 99);

	CHECK(letohl(msg.id) == CLEAR_PROMPT_STATUS_MESSAGE);
	CHECK(letohl(msg.length) == SCCP_MSG_LEN_FROM_DATA_LEN(8));
	CHECK(letohl(msg.data.clearprompt.lineInstance) == 1);
	CHECK(letohl(msg.data.clearprompt.callInstance) == 99);
}

static void test_ringer_mode(void)
{
	struct sccp_msg msg;

	sccp_msg_ringer_mode(&msg, SCCP_RING_INSIDE, 2, 1234);

	CHECK(letohl(msg.id) == SET_RINGER_MESSAGE);
	CHECK(letohl(msg.length) == SCCP_MSG_LEN_FROM_DATA_LEN(20));
	CHECK(letohl(msg.data.setringer.ringerMode) == SCCP_RING_INSIDE);
	CHECK(letohl(msg.data.setringer.ringDuration) == 1);
	/* the line and call to ring, not always line 1 */
	CHECK(letohl(msg.data.setringer.lineInstance) == 2);
	CHECK(letohl(msg.data.setringer.callReference) == 1234);
}

static void test_keep_alive_ack(void)
{
	struct sccp_msg msg;

	sccp_msg_keep_alive_ack(&msg);

	CHECK(letohl(msg.id) == KEEP_ALIVE_ACK_MESSAGE);
	CHECK(letohl(msg.length) == SCCP_MSG_LEN_FROM_DATA_LEN(0));
}

static void test_msg_id_str(void)
{
	CHECK(!strcmp(sccp_msg_id_str(DISPLAY_PROMPT_STATUS_MESSAGE), "display prompt status"));
	CHECK(!strcmp(sccp_msg_id_str(CLEAR_PROMPT_STATUS_MESSAGE), "clear prompt status"));
	CHECK(!strcmp(sccp_msg_id_str(HOOKFLASH_MESSAGE), "hookflash"));
}

static void test_device_type_str(void)
{
	CHECK(!strcmp(sccp_device_type_str(SCCP_DEVICE_6941), "6941"));
	CHECK(!strcmp(sccp_device_type_str(SCCP_DEVICE_6901), "6901"));
	CHECK(!strcmp(sccp_device_type_str(SCCP_DEVICE_6961), "6961"));
	CHECK(!strcmp(sccp_device_type_str(SCCP_DEVICE_7960), "7960"));
}

/*
 * Write raw bytes to a pipe and pop messages with the deserializer.
 */
static void test_deserializer(void)
{
	struct sccp_deserializer dzer;
	struct sccp_msg out;
	struct sccp_msg *msg;
	uint32_t header[3];
	int fds[2];

	if (pipe(fds)) {
		CHECK(0);
		return;
	}

	sccp_deserializer_init(&dzer, fds[0]);

	/* a keep alive followed by half of a register message */
	header[0] = htolel(4);
	header[1] = 0;
	header[2] = htolel(KEEP_ALIVE_MESSAGE);
	CHECK(write(fds[1], header, sizeof(header)) == sizeof(header));

	memset(&out, 0, sizeof(out));
	out.length = htolel(SCCP_MSG_LEN_FROM_DATA_LEN(sizeof(struct register_message)));
	out.id = htolel(REGISTER_MESSAGE);
	strcpy(out.data.reg.name, "SEP00260B5D4E31");
	out.data.reg.type = htolel(SCCP_DEVICE_6941);
	out.data.reg.protoVersion = 17;
	CHECK(write(fds[1], &out, 20) == 20);

	CHECK(sccp_deserializer_read(&dzer) == 0);
	CHECK(sccp_deserializer_pop(&dzer, &msg) == 0);
	CHECK(letohl(msg->id) == KEEP_ALIVE_MESSAGE);
	CHECK(sccp_deserializer_pop(&dzer, &msg) == SCCP_DESERIALIZER_NOMSG);

	/* the rest of the register message */
	CHECK(write(fds[1], (char *) &out + 20, SCCP_MSG_TOTAL_LEN_FROM_LEN(letohl(out.length)) - 20) > 0);
	CHECK(sccp_deserializer_read(&dzer) == 0);
	CHECK(sccp_deserializer_pop(&dzer, &msg) == 0);
	CHECK(letohl(msg->id) == REGISTER_MESSAGE);
	CHECK(!strcmp(msg->data.reg.name, "SEP00260B5D4E31"));
	CHECK(letohl(msg->data.reg.type) == SCCP_DEVICE_6941);
	CHECK(msg->data.reg.protoVersion == 17);

	/* a message with a length smaller than the minimum is malformed */
	header[0] = htolel(0);
	CHECK(write(fds[1], header, sizeof(header)) == sizeof(header));
	CHECK(sccp_deserializer_read(&dzer) == 0);
	CHECK(sccp_deserializer_pop(&dzer, &msg) == SCCP_DESERIALIZER_MALFORMED);

	close(fds[1]);
	close(fds[0]);
}

int main(void)
{
	test_button_template_res_legacy_size();
	test_button_template_res_large();
	test_display_prompt_status();
	test_clear_prompt_status();
	test_ringer_mode();
	test_keep_alive_ack();
	test_msg_id_str();
	test_device_type_str();
	test_deserializer();

	printf("%d checks, %d failures\n", checks, failures);

	return failures ? 1 : 0;
}
