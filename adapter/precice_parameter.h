#ifndef PRECICE_PARAMETER_H
#define PRECICE_PARAMETER_H

#include <deal.II/base/parameter_handler.h>

using namespace dealii;

/**
 * This class declares all preCICE parameters, which can be specified in the
 * parameter file. The subsection abut preCICE configurations is directly
 * interlinked to the Adapter class.
 */
namespace Parameters
{ /**
   * @brief PreciceAdapterConfiguration: Specifies preCICE related information.
   *        A lot of these information need to be consistent with the
   *        precice-config.xml file.
   */
  struct PreciceAdapterConfiguration
  {
    std::string scenario;
    std::string config_file;
    std::string participant_name;
    std::string mesh_name;
    std::string read_mesh_name;
    std::string write_mesh_name;
    int         write_sampling;
    std::string read_data_name;
    std::string write_data_name;

    static void
    declare_parameters(ParameterHandler &prm);

    void
    parse_parameters(ParameterHandler &prm);
  };


  void
  PreciceAdapterConfiguration::declare_parameters(ParameterHandler &prm)
  {
    prm.enter_subsection("precice configuration");
    {
      prm.declare_entry("Scenario",
                        "FSI3",
                        Patterns::Selection("FSI3|PF"),
                        "Cases: FSI3 or PF for perpendicular flap");
      prm.declare_entry("precice config-file",
                        "precice-config.xml",
                        Patterns::Anything(),
                        "Name of the precice configuration file");
      prm.declare_entry(
        "Participant name",
        "dealiisolver",
        Patterns::Anything(),
        "Name of the participant in the precice-config.xml file");
      prm.declare_entry(
        "Mesh name",
        "default",
        Patterns::Anything(),
        "Name of the coupling mesh in the precice-config.xml file");
      prm.declare_entry(
        "Read mesh name",
        "default",
        Patterns::Anything(),
        "Name of the read coupling mesh in the precice-config.xml file");
      prm.declare_entry(
        "Write mesh name",
        "default",
        Patterns::Anything(),
        "Name of the write coupling mesh in the precice-config.xml file");
      Patterns::Integer pattern(0);
      prm.declare_entry("Write sampling",
                        Patterns::Tools::Convert<int>::to_string(
                          std::numeric_limits<int>::max(), pattern.clone()),
                        pattern,
                        "Nodes per coupling face of the write mesh");
      prm.declare_entry("Read data name",
                        "received-data",
                        Patterns::Anything(),
                        "Name of the read data in the precice-config.xml file");
      prm.declare_entry(
        "Write data name",
        "calculated-data",
        Patterns::Anything(),
        "Name of the write data in the precice-config.xml file");
    }
    prm.leave_subsection();
  }

  void
  PreciceAdapterConfiguration::parse_parameters(ParameterHandler &prm)
  {
    prm.enter_subsection("precice configuration");
    {
      scenario         = prm.get("Scenario");
      config_file      = prm.get("precice config-file");
      participant_name = prm.get("Participant name");
      mesh_name        = prm.get("Mesh name");
      read_mesh_name   = prm.get("Read mesh name");
      write_mesh_name  = prm.get("Write mesh name");
      write_sampling   = prm.get_integer("Write sampling");
      read_data_name   = prm.get("Read data name");
      write_data_name  = prm.get("Write data name");
    }

    const std::string error_message(
      "Either specify a 'Mesh name', which will be applied to the read and write mesh (data location)"
      " or a separate 'Read Mesh name' and a 'Write mesh name' in order to enable more mapping frindly "
      "specialized data locations at the interface. Specifying both or none of these "
      "options is invalid. Make sure you adjust your configuration file '" +
      config_file + "' according to your settings.");

    if (mesh_name != "default")
      {
        AssertThrow((mesh_name != read_mesh_name), ExcMessage(error_message));

        read_mesh_name  = mesh_name;
        write_mesh_name = mesh_name;
      }
    else
      {
        AssertThrow(("default" != read_mesh_name) &&
                      ("default" != write_mesh_name),
                    ExcMessage(error_message));

        AssertThrow("default" == mesh_name, ExcMessage(error_message));
      }

    prm.leave_subsection();
  }
} // namespace Parameters


#endif // PRECICE_PARAMETER_H
