#pragma once

const constexpr short MESSAGE_HEAD_LEN = 8;
const constexpr int MESSAGE_LENGTH_LIMIT = 65527;

struct message_head {
  int message_id;
  int message_len;
};
