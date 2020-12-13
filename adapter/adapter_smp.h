#ifndef ADAPTER_H
#define ADAPTER_H

#include <deal.II/base/exceptions.h>
#include <deal.II/base/std_cxx20/iota_view.h>

#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/mapping_q_generic.h>

#include <precice/SolverInterface.hpp>

#include "q_equidistant.h"
#include "time.h"

namespace Adapter
{
  using namespace dealii;

  /**
   * The Adapter class keeps all functionalities to couple deal.II to other
   * solvers with preCICE i.e. data structures are set up, necessary information
   * is passed to preCICE etc.
   */
  template <int dim, typename VectorType, typename ParameterClass>
  class Adapter
  {
  public:
    /**
     * @brief      Constructor, which sets up the precice Solverinterface
     *
     * @param[in]  parameters Parameter class, which hold the data specified
     *             in the parameters.prm file
     * @param[in]  deal_boundary_interface_id Boundary ID of the triangulation,
     *             which is associated with the coupling interface.
     */
    Adapter(const ParameterClass &parameters,
            const unsigned int    dealii_boundary_interface_id);

    /**
     * @brief      Initializes preCICE and passes all relevant data to preCICE
     *
     * @param[in]  dof_handler Initialized dof_handler
     * @param[in]  deal_to_precice Data, which should be given to preCICE and
     *             exchanged with other participants. Wether this data is
     *             required already in the beginning depends on your
     *             individual configuration and preCICE determines it
     *             automatically. In many cases, this data will just represent
     *             your initial condition.
     * @param[out] precice_to_deal Data, which is received from preCICE/ from
     *             other participants. Wether this data is useful already in
     *             the beginning depends on your individual configuration and
     *             preCICE determines it automatically. In many cases, this
     *             data will just represent the initial condition of other
     *             participants.
     */
    void
    initialize(const DoFHandler<dim> &    dof_handler,
               const Mapping<dim> &       mapping,
               const Quadrature<dim - 1> &write_quadrature,
               const Quadrature<dim - 1> &read_quadrature,
               const VectorType &         dealii_to_precice);

    /**
     * @brief      Advances preCICE after every timestep, converts data formats
     *             between preCICE and dealii
     *
     * @param[in]  deal_to_precice Same data as in @p initialize_precice() i.e.
     *             data, which should be given to preCICE after each time step
     *             and exchanged with other participants.
     * @param[out] precice_to_deal Same data as in @p initialize_precice() i.e.
     *             data, which is received from preCICE/other participants
     *             after each time step and exchanged with other participants.
     * @param[in]  computed_timestep_length Length of the timestep used by
     *             the solver.
     */
    void
    advance(const VectorType &         dealii_to_precice,
            const DoFHandler<dim> &    dof_handler,
            const Mapping<dim> &       mapping,
            const Quadrature<dim - 1> &write_quadrature,
            const double               computed_timestep_length);

    /**
     * @brief      Saves current state of time dependent variables in case of an
     *             implicit coupling
     *
     * @param[in]  state_variables Vector containing all variables to store as
     *             reference
     *
     * @note       This function only makes sense, if it is used with
     *             @p reload_old_state_if_required. Therefore, the order, in which the
     *             variables are passed into the vector must be the same for
     *             both functions.
     * @note       The absolute time has no impact on the computation, but on the output.
     *             Therefore, we call here in the @p Time class a method to store the
     *             current time and reload it later. This is necessary, in
     *             case your solver is subcycling.
     */
    void
    save_current_state_if_required(
      const std::vector<VectorType *> &state_variables,
      Time &                           time_class);

    /**
     * @brief      Reloads the previously stored variables in case of an implicit
     *             coupling. The current implementation supports subcycling,
     *             i.e. previously refers o the last time
     *             @p save_current_state_if_required() has been called.
     *
     * @param[out] state_variables Vector containing all variables to reload
     *             as reference
     *
     * @note       This function only makes sense, if the state variables have been
     *             stored by calling @p save_current_state_if_required. Therefore,
     *             the order, in which the variables are passed into the
     *             vector must be the same for both functions.
     */
    void
    reload_old_state_if_required(std::vector<VectorType *> &state_variables,
                                 Time &                     time_class);

    /**
     * @brief public precice solverinterface
     */

    precice::SolverInterface precice;

    // Boundary ID of the deal.II mesh, associated with the coupling
    // interface. The variable is public and should be used during grid
    // generation, but is also involved during system assembly. The only thing,
    // one needs to make sure is, that this ID is not given to another part of
    // the boundary e.g. clamped one.
    const unsigned int dealii_boundary_interface_id;


    void
    read_on_quadrature_point(std::array<double, dim> &data,
                             const unsigned int       q_index) const;

    /**
     * @brief read_on_quadrature_point_with_ID Given the ID we want to access, returns the local data on each quad poin
     * @param data
     * @param q_index
     */
    void
    read_on_quadrature_point_with_ID(Tensor<1, dim> &   data,
                                     const unsigned int q_index) const;

    auto
    begin_interface_IDs() const
    {
      return read_nodes_ids.begin();
    }

    unsigned int
    get_node_id(const unsigned int face_id) const;

  private:
    // preCICE related initializations
    // These variables are specified and read from the parameter file
    const std::string read_mesh_name;
    const std::string write_mesh_name;
    const std::string read_data_name;
    const std::string write_data_name;

    // To be adjusted for MPI parallelized codes
    static constexpr unsigned int this_mpi_process = 0;
    static constexpr unsigned int n_mpi_processes  = 1;

    // These IDs are given by preCICE during initialization
    int read_mesh_id;
    int write_mesh_id;
    int read_data_id;
    int write_data_id;


    // Data containers which are passed to preCICE in an appropriate preCICE
    // specific format
    std::vector<int>                     read_nodes_ids;
    std::vector<int>                     write_nodes_ids;
    std::map<unsigned int, unsigned int> read_id_map;
    std::vector<double>                  read_data;

    // Container to store time dependent data in case of an implicit coupling
    std::vector<VectorType> old_state_data;
    double                  old_time_value;

    void
    write_all_quadrature_nodes(const VectorType &         data,
                               const Mapping<dim> &       mapping,
                               const DoFHandler<dim> &    dof_handler,
                               const Quadrature<dim - 1> &write_quadrature);

    void
    set_mesh_vertices(const Mapping<dim> &       mapping,
                      const DoFHandler<dim> &    dof_handler,
                      const Quadrature<dim - 1> &quadrature,
                      const bool                 is_read_mesh);
  };



  template <int dim, typename VectorType, typename ParameterClass>
  Adapter<dim, VectorType, ParameterClass>::Adapter(
    const ParameterClass &parameters,
    const unsigned int    dealii_boundary_interface_id)
    : precice(parameters.participant_name,
              parameters.config_file,
              this_mpi_process,
              n_mpi_processes)
    , dealii_boundary_interface_id(dealii_boundary_interface_id)
    , read_mesh_name(parameters.read_mesh_name)
    , write_mesh_name(parameters.write_mesh_name)
    , read_data_name(parameters.read_data_name)
    , write_data_name(parameters.write_data_name)
  {}



  template <int dim, typename VectorType, typename ParameterClass>
  void
  Adapter<dim, VectorType, ParameterClass>::initialize(
    const DoFHandler<dim> &    dof_handler,
    const Mapping<dim> &       mapping,
    const Quadrature<dim - 1> &write_quadrature,
    const Quadrature<dim - 1> &read_quadrature,
    const VectorType &         dealii_to_precice)
  {
    AssertThrow(
      dim == precice.getDimensions(),
      ExcMessage("The dimension of your solver needs to be consistent with the "
                 "dimension specified in your precice-config file. In case you "
                 "run one of the tutorials, the dimension can be specified via "
                 "cmake -D DIM=dim ."));

    AssertThrow(dim > 1, ExcNotImplemented());

    // get precice specific IDs from precice and store them in the class
    // they are later needed for data transfer
    read_mesh_id  = precice.getMeshID(read_mesh_name);
    read_data_id  = precice.getDataID(read_data_name, read_mesh_id);
    write_mesh_id = precice.getMeshID(write_mesh_name);
    write_data_id = precice.getDataID(write_data_name, write_mesh_id);

    set_mesh_vertices(mapping, dof_handler, write_quadrature, false);
    set_mesh_vertices(mapping, dof_handler, read_quadrature, true);

    std::cout << "\t Number of read nodes:     " << read_nodes_ids.size()
              << std::endl;
    std::cout << "\t Number of write nodes:     " << write_nodes_ids.size()
              << std::endl;

    read_data.resize(read_nodes_ids.size() * dim);

    // Initialize preCICE internally
    precice.initialize();

    // write initial writeData to preCICE if required
    if (precice.isActionRequired(precice::constants::actionWriteInitialData()))
      {
        write_all_quadrature_nodes(dealii_to_precice,
                                   mapping,
                                   dof_handler,
                                   write_quadrature);

        precice.markActionFulfilled(
          precice::constants::actionWriteInitialData());

        precice.initializeData();
      }

    precice.readBlockVectorData(read_data_id,
                                read_nodes_ids.size(),
                                read_nodes_ids.data(),
                                read_data.data());
  }



  template <int dim, typename VectorType, typename ParameterClass>
  void
  Adapter<dim, VectorType, ParameterClass>::advance(
    const VectorType &         dealii_to_precice,
    const DoFHandler<dim> &    dof_handler,
    const Mapping<dim> &       mapping,
    const Quadrature<dim - 1> &write_quadrature,
    const double               computed_timestep_length)
  {
    if (precice.isWriteDataRequired(computed_timestep_length))
      write_all_quadrature_nodes(dealii_to_precice,
                                 mapping,
                                 dof_handler,
                                 write_quadrature);

    // Here, we need to specify the computed time step length and pass it to
    // preCICE
    precice.advance(computed_timestep_length);

    if (precice.isReadDataAvailable())
      precice.readBlockVectorData(read_data_id,
                                  read_nodes_ids.size(),
                                  read_nodes_ids.data(),
                                  read_data.data());

    // Alternative, if you don't want to store the indices
    //    const int rsize = (1 / dim) * read_data.size();
    //    if (precice.isReadDataAvailable())
    //      for (const auto i : std_cxx20::ranges::iota_view<int, int>{0,
    //      rsize})
    //        precice.readVectorData(read_data_id, i, &read_data[i * dim]);
  }



  template <int dim, typename VectorType, typename ParameterClass>
  void
  Adapter<dim, VectorType, ParameterClass>::save_current_state_if_required(
    const std::vector<VectorType *> &state_variables,
    Time &                           time_class)
  {
    // First, we let preCICE check, whether we need to store the variables.
    // Then, the data is stored in the class
    if (precice.isActionRequired(
          precice::constants::actionWriteIterationCheckpoint()))
      {
        old_state_data.resize(state_variables.size());

        for (uint i = 0; i < state_variables.size(); ++i)
          old_state_data[i] = *(state_variables[i]);

        old_time_value = time_class.current();

        precice.markActionFulfilled(
          precice::constants::actionWriteIterationCheckpoint());
      }
  }



  template <int dim, typename VectorType, typename ParameterClass>
  void
  Adapter<dim, VectorType, ParameterClass>::reload_old_state_if_required(
    std::vector<VectorType *> &state_variables,
    Time &                     time_class)
  {
    // In case we need to reload a state, we just take the internally stored
    // data vectors and write then in to the input data
    if (precice.isActionRequired(
          precice::constants::actionReadIterationCheckpoint()))
      {
        Assert(state_variables.size() == old_state_data.size(),
               ExcMessage(
                 "state_variables are not the same as previously saved."));

        for (uint i = 0; i < state_variables.size(); ++i)
          *(state_variables[i]) = old_state_data[i];

        // Here, we expect the time class to offer an option to specify a
        // given time value.
        time_class.set_absolute_time(old_time_value);

        precice.markActionFulfilled(
          precice::constants::actionReadIterationCheckpoint());
      }
  }

  template <int dim, typename VectorType, typename ParameterClass>
  void
  Adapter<dim, VectorType, ParameterClass>::write_all_quadrature_nodes(
    const VectorType &         data,
    const Mapping<dim> &       mapping,
    const DoFHandler<dim> &    dof_handler,
    const Quadrature<dim - 1> &write_quadrature)
  {
    FEFaceValues<dim>           fe_face_values(mapping,
                                     dof_handler.get_fe(),
                                     write_quadrature,
                                     update_values);
    std::vector<Vector<double>> quad_values(write_quadrature.size(),
                                            Vector<double>(dim));
    std::array<double, dim>     local_data;
    auto                        index = write_nodes_ids.begin();

    for (const auto &cell : dof_handler.active_cell_iterators())
      for (const auto &face : cell->face_iterators())
        if (face->at_boundary() == true &&
            face->boundary_id() == dealii_boundary_interface_id)
          {
            fe_face_values.reinit(cell, face);
            fe_face_values.get_function_values(data, quad_values);

            // Alternative: write data of a cell as a whole block using
            // writeBlockVectorData
            for (const auto f_q_point :
                 fe_face_values.quadrature_point_indices())
              {
                Assert(index != write_nodes_ids.end(), ExcInternalError());
                // TODO: Check if the additional array is necessary. Maybe we
                // can directly use quad_values[f_q_point].data() for preCICE
                for (uint d = 0; d < dim; ++d)
                  local_data[d] = quad_values[f_q_point][d];

                precice.writeVectorData(write_data_id,
                                        *index,
                                        local_data.data());

                ++index;
              }
          }
  }



  template <int dim, typename VectorType, typename ParameterClass>
  void
  Adapter<dim, VectorType, ParameterClass>::read_on_quadrature_point(
    std::array<double, dim> &data,
    const unsigned int       index) const
  {
    // TODO: Check if the if statement still makes sense
    //      if (precice.isReadDataAvailable())
    precice.readVectorData(read_data_id, index, data.data());
  }



  template <int dim, typename VectorType, typename ParameterClass>
  void
  Adapter<dim, VectorType, ParameterClass>::read_on_quadrature_point_with_ID(
    Tensor<1, dim> &   data,
    const unsigned int ID_index) const
  {
    // Assert the accesses index
    AssertIndexRange(ID_index * dim + (dim - 1), read_data.size());
    for (uint d = 0; d < dim; ++d)
      data[d] = read_data[ID_index * dim + d];
  }



  template <int dim, typename VectorType, typename ParameterClass>
  void
  Adapter<dim, VectorType, ParameterClass>::set_mesh_vertices(
    const Mapping<dim> &       mapping,
    const DoFHandler<dim> &    dof_handler,
    const Quadrature<dim - 1> &quadrature,
    const bool                 is_read_mesh)
  {
    const unsigned int mesh_id = is_read_mesh ? read_mesh_id : write_mesh_id;
    auto &interface_nodes_ids = is_read_mesh ? read_nodes_ids : write_nodes_ids;


    // TODO: Find a suitable guess for the number of interface points (optional)
    interface_nodes_ids.reserve(20);
    std::array<double, dim> vertex;
    FEFaceValues<dim>       fe_face_values(mapping,
                                     dof_handler.get_fe(),
                                     quadrature,
                                     update_quadrature_points);

    for (const auto &cell : dof_handler.active_cell_iterators())
      for (const auto &face : cell->face_iterators())
        if (face->at_boundary() == true &&
            face->boundary_id() == dealii_boundary_interface_id)
          {
            fe_face_values.reinit(cell, face);

            // Create a map for shared parallelism
            if (is_read_mesh)
              read_id_map[cell->face_index(cell->face_iterator_to_index(
                face))] = interface_nodes_ids.size();

            for (const auto f_q_point :
                 fe_face_values.quadrature_point_indices())
              {
                const auto &q_point =
                  fe_face_values.quadrature_point(f_q_point);
                for (uint d = 0; d < dim; ++d)
                  vertex[d] = q_point[d];

                interface_nodes_ids.emplace_back(
                  precice.setMeshVertex(mesh_id, vertex.data()));
              }
          }
  }

  template <int dim, typename VectorType, typename ParameterClass>
  unsigned int
  Adapter<dim, VectorType, ParameterClass>::get_node_id(
    const unsigned int face_id) const
  {
    return read_id_map.at(face_id);
  }

} // namespace Adapter
#endif // ADAPTER_H
