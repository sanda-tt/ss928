const DEVICE_ID = "bag001";
const HISTORY_LIMIT = 100;

const success = (data) => ({ ok: true, data });
const failure = (code) => ({ ok: false, error: { code, message: code } });

const createAppApi = ({ repository }) => {
  if (!repository) {
    throw new Error("App API requires a repository");
  }
  return {
    async handle(event) {
      const action = event && event.action;
      try {
        if (action === "getLatestStatus") {
          return success({ status: await repository.getStatus(DEVICE_ID) });
        }
        if (action === "getTrackPoints") {
          return success({ items: await repository.listTrackPoints(DEVICE_ID, HISTORY_LIMIT) });
        }
        if (action === "getAlarmHistory") {
          return success({ items: await repository.listAlarmHistory(DEVICE_ID, HISTORY_LIMIT) });
        }
        return failure("UNSUPPORTED_ACTION");
      } catch (error) {
        return failure("INTERNAL_ERROR");
      }
    }
  };
};

module.exports = { DEVICE_ID, HISTORY_LIMIT, createAppApi };
